#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "mock_client/proto/extractor.pb.h" // Generated by protoc
#include "mock_client/proto/extractor.grpc.pb.h" // Generated by protoc-gen-grpc
#include "pipeline.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using extractor::ExtractorService;
// Removed unused old message types (FlightRow, ReservaRow, PesquisaRow)
// as they are no longer directly part of AllDataSend in the new proto.
using extractor::AllDataSend;
using extractor::AllDataResponse;
#include <vector>
#include <sstream> // Required for std::istringstream

std::string getFirstNLines(const std::string& csv_content, int n) {
    std::istringstream iss(csv_content); // Treat the string as an input stream
    std::string line;
    std::string result_csv;
    int line_count = 0;

    while (std::getline(iss, line) && line_count < n) {
        result_csv += line + "\n"; // Append the line and a newline character
        line_count++;
    }
    return result_csv;
}

// Example usage within your context:
// const std::string& voos_csv_content = request->voos();
// std::string first_200_voos_lines = getFirstNLines(voos_csv_content, 200);
// std::cout << "First 200 lines of voos_csv_content:\n" << first_200_voos_lines << std::endl;
// Logic and data behind the server's behavior.
class ExtractorServiceImpl final : public ExtractorService::Service {
    Status GetAllData(ServerContext* context, const AllDataSend* request, AllDataResponse* response) override {
        // Access the string content from the request
        const std::string& voos_csv_content = request->voos();
        const std::string& reservas_csv_content = request->reservas();
        const std::string& pesquisas_csv_content = request->pesquisas();
        
        // --- Print the content of the received CSV strings ---
        // std::cout << "Server received AllDataSend:" << std::endl;
        // std::cout << "--- Voos CSV Content ---" << std::endl;
        // std::cout << voos_csv_content << std::endl;
        // std::cout << "--- Reservas CSV Content ---" << std::endl;
        // std::cout << reservas_csv_content << std::endl;
        // std::cout << "--- Pesquisas CSV Content ---" << std::endl;
        // std::cout << pesquisas_csv_content << std::endl;
        // std::cout << "-----------------------------------" << std::endl;

        

        std::vector<int> stats_response = pipeline(reservas_csv_content,
            voos_csv_content,
            pesquisas_csv_content);
            
        // --- Set all stats to 5 as requested ---
        response->set_stats1(stats_response[0]);
        response->set_stats2(stats_response[1]);
        response->set_stats3(stats_response[2]);
        response->set_stats4(stats_response[3]);
        response->set_stats5(stats_response[4]);

        std::cout << "Server sending AllDataResponse with all stats set to 5:" << std::endl;
        std::cout << "  Stats1: " << response->stats1() << std::endl;
        std::cout << "  Stats2: " << response->stats2() << std::endl;
        std::cout << "  Stats3: " << response->stats3() << std::endl;
        std::cout << "  Stats4: " << response->stats4() << std::endl;
        std::cout << "  Stats5: " << response->stats5() << std::endl;
        std::cout << "-----------------------------------" << std::endl;

        return Status::OK;
    }
};

void RunServer() {
    std::string server_address("localhost:50051");
    ExtractorServiceImpl service;

    ServerBuilder builder;
    // Set max message sizes to handle large CSV strings (e.g., 50 MB)
    builder.SetMaxReceiveMessageSize(50 * 1024 * 1024);
    builder.SetMaxSendMessageSize(50 * 1024 * 1024);
    
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll perform
    // the actual work.
    builder.RegisterService(&service);
    // Finally, assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}