#include <iostream>
#include <thread>
#include <vector>
#include <ctime>
#include <chrono>
#include <atomic>

#include "zmq.hpp"
#include "Messages.h"
#include "MessageUtilities.h"
#include "ServerMessageUtilities.h"
#include "ServerStatusMessage.h"
#include "RemoteHopsanClient.h"

#include "ServerHandler.h"
#include "common.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

using namespace std;
using namespace std::chrono;

std::string nowDateTime()
{
    std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buff[64];
    std::strftime(buff, sizeof(buff), "%b %d %H:%M:%S", std::localtime(&now_time));
    return std::string(&buff[0]);
}

ServerHandler gServerHandler;

// Prepare our context and socket
#ifdef _WIN32
zmq::context_t gContext(1, 63);
#else
zmq::context_t gContext(1);
#endif

static int s_interrupted = 0;
#ifdef _WIN32
BOOL WINAPI consoleCtrlHandler( DWORD dwCtrlType )
{
    // what to do here?
    s_interrupted = 1;
    return TRUE;
}
#else
static void s_signal_handler(int signal_value)
{
    s_interrupted = 1;
}

static void s_catch_signals(void)
{
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}
#endif

const int gMaxNumRunningRefreshServerStatusThreads = 20;
std::atomic<int> gNumRunningRefreshServerStatusThreads{0};
void refreshServerStatus(size_t serverId)
{
    gNumRunningRefreshServerStatusThreads++;
    RemoteHopsanClient hopsanClient(gContext);
    if (hopsanClient.areSocketsValid())
    {
        ServerInfo server = gServerHandler.getServer(serverId);
        if (server.isValid())
        {
            cout << PRINTSERVER << nowDateTime() << " Requesting status from server: " << serverId << endl;
            hopsanClient.connectToServer(server.ip, server.port);
            ServerStatusT status;
            bool rc = hopsanClient.requestStatus(status);
            if (rc)
            {
                cout << PRINTSERVER << nowDateTime() << " Server: " << serverId << " is responding!" << endl;
                server.lastCheckTime = steady_clock::now();
                server.isReady = status.isReady;
                gServerHandler.updateServerInfo(server);
            }
            else
            {
                cout << PRINTSERVER << nowDateTime() << " Server: " << serverId << " is NOT responding!" << endl;
                gServerHandler.removeServer(serverId);
                //! @todo what if network temporarily down
            }
        }
    }
    gNumRunningRefreshServerStatusThreads--;
}

void refreshServerThread()
{
    const double maxAgeSeconds=60;
    cout << PRINTSERVER << nowDateTime() << " Starting server refresh thread!" << endl;

    while (true)
    {
        // Check break condition
        if (s_interrupted)
        {
            break;
        }

        // Extract oldest servers
        //! @todo maybe extract age as well, under the same lock
        list<int> server_ids;

        int nRunning = gNumRunningRefreshServerStatusThreads;
        cout << "Num RefreshThreads running: " << nRunning << endl;
        cout << "Num Servers: " << gServerHandler.numServers() << endl;
        // If no servers are available, then sleep for a while
        if (gServerHandler.numServers() == 0)
        {
            std::chrono::milliseconds ms{int(floor(maxAgeSeconds*1000))};
            cout << "No servers sleeping for: " << ms.count() << " milliseconds" << endl;
            std::this_thread::sleep_for(ms);
        }
        // Else try to get the oldest servers
        else
        {

            server_ids = gServerHandler.getOldestServers(gMaxNumRunningRefreshServerStatusThreads-nRunning);

            // If we did not get any then we have maxed out our num refresh threads, so sleep a short while before we try again
            if (server_ids.empty() && (gMaxNumRunningRefreshServerStatusThreads-nRunning)==0 )
            {
                std::chrono::milliseconds ms{1000};
                cout << "Max num refresh threads running, sleeping for: " << ms.count() << " milliseconds" << endl;
                std::this_thread::sleep_for(ms);
            }
        }

        // Check break condition
        if (s_interrupted)
        {
            break;
        }

        // Now process them, Oldest should be first in the list
        for (size_t server_id : server_ids)
        {
            // Check break condition
            if (s_interrupted)
            {
                break;
            }

            duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - gServerHandler.getServerAge(server_id));
            if (time_span.count() < maxAgeSeconds)
            {
                // Sleep until its time
                std::chrono::milliseconds ms{int(floor((maxAgeSeconds-time_span.count())*1000))};
                cout << "Non needs update, Sleeping for: " << ms.count() << " milliseconds" << endl;
                std::this_thread::sleep_for(ms);
            }
            else
            {
                std::chrono::milliseconds ms{50};
                std::this_thread::sleep_for(ms);
            }

            // Spawn refresh threads (the threds will dealocate themselves when done)
            std::thread (refreshServerStatus, server_id ).detach();
        }
    }
    cout << PRINTSERVER << nowDateTime() << " Exiting server refresh thread!" << endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << PRINTSERVER << nowDateTime() << " Error: you must specify what base port to use!" << endl;
        return 1;
    }
    string myPort = argv[1];

    cout << PRINTSERVER << nowDateTime() << " Listening on port: " << myPort  << endl;


    zmq::socket_t socket (gContext, ZMQ_REP);
    int linger_ms = 1000;
    socket.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));

    socket.bind( makeZMQAddress("*", myPort).c_str() );

#ifdef _WIN32
    SetConsoleCtrlHandler( consoleCtrlHandler, TRUE );
#else
    s_catch_signals();
#endif

    // Start refresh thread
    std::thread refreshThread = std::thread( refreshServerThread );

    // Start main thread
    while (true)
    {
        // Wait for next request from client
        zmq::message_t message;
        if(receiveWithTimeout(socket, 30000, message))
        {
            size_t offset=0;
            bool idParseOK;
            size_t msg_id = getMessageId(message, offset, idParseOK);

            if (msg_id == S_Available)
            {
                bool parseOK;
                SM_Available_t sm = unpackMessage<SM_Available_t>(message,offset,parseOK);

                ServerInfo si;
                si.ip = sm.ip;
                si.port = sm.port;
                si.lastCheckTime = steady_clock::time_point(); // Epoch time

                if (gServerHandler.getServerMatching(sm.ip, sm.port) < 0)
                {
                    gServerHandler.addServer(si);
                    sendServerAck(socket);
                    // Get the oldest one, (should be the one we just added, since it need emmediate update)
                    std::list<int> list = gServerHandler.getOldestServers(1);
                    // Start a refresh thread for this one server, unless we have the maximum number of threads running
                    // In that case we let the ordinary refresh thread handle this server
                    if (gNumRunningRefreshServerStatusThreads < gMaxNumRunningRefreshServerStatusThreads)
                    {
                        std::thread ( refreshServerStatus, list.front() ).detach();
                    }
                }
                else
                {
                    sendServerNAck(socket, "Address is already registered");
                }
            }
            else if (msg_id == S_Closing)
            {
                bool parseOK;
                SM_Available_t sm = unpackMessage<SM_Available_t>(message,offset,parseOK);
                cout << PRINTSERVER << nowDateTime() << " Server at IP: " << sm.ip << ":" << sm.port << " is closing!" << endl;

                // lookup server
                //! @todo need to give servers a unique id to avoid others from beeing able to close them
                int id = gServerHandler.getServerMatching(sm.ip, sm.port);
                if (id >= 0)
                {
                    gServerHandler.removeServer(id);
                    sendServerAck(socket);
                }
                else
                {
                    sendServerNAck(socket, "You are not registered");
                }
            }
            else if (msg_id == C_ReqServerMachines)
            {
                //! @todo maybe refresh all before checking

                //! @todo be smart
                bool parseOK;
                CM_ReqServerMachines_t req = unpackMessage<CM_ReqServerMachines_t>(message,offset,parseOK);
                cout << PRINTSERVER << nowDateTime() << " Got server machines request" << endl;
                auto ids = gServerHandler.getServersFasterThen(req.maxBenchmarkTime, req.numMachines);
                vector<string> ips, ports;
                for (auto id : ids)
                {
                    ServerInfo server = gServerHandler.getServer(id);
                    if (server.isValid())
                    {
                        ips.push_back(server.ip);
                        ports.push_back(server.port);
                    }
                }

                MSM_ReqServerMachines_Reply_t reply;
                reply.ips = ips;
                reply.ports = ports;

                sendServerMessage<MSM_ReqServerMachines_Reply_t>(socket, S_ReqServerMachines_Reply, reply);
            }
            else if (!idParseOK)
            {
                cout << PRINTSERVER << nowDateTime() << " Error: Could not parse message id" << std::endl;
            }

        }

        // Every time we get here we should refresh status from servers where status is old
        // Note1! You should make sure that the max age here is shorter then the "Report to master server" timer in the actual server
        // Otherwise you will end up with the server trying to reregister because no status has been requested
        //
        // Note2! You should limit the number of servers to attempt refresh on,
        //        else you will run out of file descriptors if to many request threads are made at the same time
//        double maxAgeSeconds=60;
//        //! @todo here we have a serious problem if we have too many servers, we will not have time to refresh them all ever
//        //! @todo refreshing should be in a different thread really, or we could only refresh when someone makes a request
//        std::list<size_t> refreshList = gServerHandler.getServersToRefresh(maxAgeSeconds, 100);
//        cout << PRINTSERVER << nowDateTime() << " Debug: refreshList.size(): " << refreshList.size() << endl;
//        for (auto &item : refreshList)
//        {
//            // Spawn refresh threads (the thredas will dealocate themselves when done)
//            std::thread (refreshServerStatus, item ).detach();
//        }

        // check quit signal
        if (s_interrupted)
        {
            cout << PRINTSERVER << nowDateTime() << " Interrupt signal received, killing server" << std::endl;
            break;
        }
    }

    s_interrupted = 1;
    cout << PRINTSERVER << nowDateTime() << " Waiting for server refresh thread..." << endl;
    refreshThread.join();

    cout << PRINTSERVER << nowDateTime() << " Closed!" << endl;
    return 0;
}
