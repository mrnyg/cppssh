#include "cppssh.h"
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <sstream>

#define NUM_THREADS 1


void reportErrors(const std::string& tag, const int channel)
{
    CppsshMessage logMessage;
    while (Cppssh::getLogMessage(channel, &logMessage))
    {
        std::cout << tag << " " << channel << " " << logMessage.message() << std::endl;
    }
}

void getOutFile(int channel, std::ofstream& outfile)
{
    std::stringstream filename;
    filename << "channel" << channel << ".log";
    outfile.open(filename.str());
}

void runConnectionTest(char* hostname, char* username, char* password)
{
    int channel;
    if (Cppssh::connect(&channel, hostname, 22, username, password, NUM_THREADS * 10) == true)
    {
        std::ofstream output;
        getOutFile(channel, output);
        std::cout << "Connected " << channel << std::endl;
        std::chrono::steady_clock::time_point txTime = std::chrono::steady_clock::now();
        int txCount = 0;
        bool sentGvim = false;
        while ((Cppssh::isConnected(channel) == true) && (std::chrono::steady_clock::now() < (txTime + std::chrono::seconds(1))))
        {
            CppsshMessage message;
            if (Cppssh::read(channel, &message) == true)
            {
                output << message.message();
            }
            if (sentGvim == false)
            {
                Cppssh::writeString(channel, "gvim\n");
                sentGvim = true;
            }

            if ((txCount < 100) && (std::chrono::steady_clock::now() > (txTime + std::chrono::milliseconds(100))))
            {
                // send ls -l every 100 milliseconds
                Cppssh::writeString(channel, "ls -l\n");
                txTime = std::chrono::steady_clock::now();
                txCount++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    else
    {
        std::cout << "Did not connect " << channel << std::endl;
    }
    reportErrors("Connect", channel);
    Cppssh::close(channel);
}

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        std::cerr << "Error: Three arguments required: " << argv[0] << " <hostname> <username> <password>" << std::endl;
        return -1;
    }

    try
    {
        Cppssh::create();
        std::vector<std::string> ciphers;
        std::vector<std::string> macs;
        Cppssh::setOptions("aes256-cbc", "hmac-md5");

        //Cppssh::generateRsaKeyPair("test", "privRsa", "pubRsa", 1024);
        //Cppssh::generateDsaKeyPair("test", "privDsa", "pubDsa", 1024);

        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; i++)
        {
            threads.push_back(std::thread(&runConnectionTest, argv[1], argv[2], argv[3]));
        }
        for (std::vector<std::thread>::iterator it = threads.begin(); it != threads.end(); it++)
        {
            (*it).join();
        }
        Cppssh::destroy();
    }
    catch (const std::exception& ex)
    {
        std::cout << "Exception: " << ex.what() << std::endl;
    }
    return 0;
}

