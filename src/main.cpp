#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

std::string getApiKeyFromEnv()
{
    std::ifstream env("../.env");
    std::string line;

    while (std::getline(env, line))
    {
        if (line.rfind("OPENAI_API_KEY=", 0) == 0)
        {
            return line.substr(std::strlen("OPENAI_API_KEY="));
        }
    }

    return "";
}

std::string getUserInput()
{
    std::cout << "You: ";
    std::string input;
    std::getline(std::cin, input);

    return input;
}

bool handleCommands(const std::string &input, nlohmann::json &messages)
{
    if (input == "/help")
    {
        std::cout << "Available commands: \n"
                  << "/help - Show this help\n"
                  << "/reset - Clear chat memory\n"
                  << "/export - Save chat to file\n"
                  << "/exit - Quit the program\n"
                  << "/sentiment - Deduce mood and emotions\n";
        return true;
    }

    if (input == "/reset")
    {
        messages = nlohmann::json::array();
        messages.push_back({{"role", "system"}, {"content", "You are a helpful assistant"}});
        std::cout << "Chat reset. \n";
        return true;
    }

    if (input == "/exit")
    {
        std::cout << "Exiting chat! \n";
        exit(0);
    }

    if (input == "/export")
    {
        std::ofstream file("chats.txt");
        if (!file.is_open())
        {
            std::cerr << "Failed to open chats.txt";
            return true;
        }

        for (const auto &message : messages)
        {
            file << message["role"] << ": " << message["content"] << "\n";
        }

        file.close();
        std::cout << "Chat exported successfully. Open chat.txt to view \n";
        return true;
    }

    if (input.rfind("/sentiment ", 0) == 0)
    {
        std::string apiKey = getApiKeyFromEnv();
        using json = nlohmann::json;
        json messages = json::array();

        std::string textToAnalyze = input.substr(11);
        json sentimentPrompt = {
            {"role", "user"},
            {"content", "Analyze the sentiment of the following message: \"" + textToAnalyze + "\""}};

        json sentimentPayload = {{"model", "gpt-4o-mini"}, {"messages", json::array({{{"role", "system"}, {"content", "You are a sentiment analysis bot. Respond only with the sentiment: Positive, Negative, Neutral, etc."}}, sentimentPrompt})}};

        cpr::Response response = cpr::Post(
            cpr::Url{"https://api.openai.com/v1/chat/completions"},
            cpr::Header{
                {"Authorization", "Bearer " + apiKey},
                {"Content-Type", "application/json"}},
            cpr::Body{sentimentPayload.dump()});

        if (response.status_code != 200)
        {
            std::cerr << "Error getting sentiment: " << response.status_code << "\n";
            std::cerr << response.text << "\n";
            return true;
        }

        json jsonResp = json::parse(response.text);

        std::string sentiment = jsonResp["choices"][0]["message"]["content"];

        std::cout << "Sentiment: " << sentiment << "\n";

        return true;
    }

    return false;
}

int main()
{
    std::cout << "\n//Week 2 GPT Chatbot Running with Memory\n\n";
    std::string apiKey = getApiKeyFromEnv();

    using json = nlohmann::json;
    json messages = json::array();
    messages.push_back({{"role", "system"}, {"content", "You are a helpful assistant"}});

    while (true)
    {
        std::string input = getUserInput();

        if (input[0] == '/')
        {
            if (handleCommands(input, messages))
            {
                continue;
            }
        }

        messages.push_back({{"role", "user"}, {"content", input}});

        json payload = {
            {"model", "gpt-4o-mini"},
            {"messages", messages}};

        cpr::Response response = cpr::Post(
            cpr::Url{"https://api.openai.com/v1/chat/completions"},
            cpr::Header{{"Authorization", "Bearer " + apiKey}, {"Content-Type", "application/json"}},
            cpr::Body{payload.dump()});

        if (response.status_code != 200)
        {
            std::cerr << "❌ Error: " << response.status_code << "\n";
            std::cerr << response.text << "\n";
            continue;
        }

        json jsonResp = json::parse(response.text);

        std::string reply = jsonResp["choices"][0]["message"]["content"];

        messages.push_back({{"role", "assistant"}, {"content", reply}});

        std::cout << "\nBot: " << reply << "\n";
    }
}
