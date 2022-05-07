#include "xmpp.h"
#include "server.h"

int main(void) {
	
    std::string xmppStrPort;
    std::cout << _("Enter the port for the XMPP: ");
    std::getline(std::cin, xmppStrPort);
	
    std::thread XMPPThread = std::thread(StartXMPP, std::stoi(xmppStrPort));

    while (!bHasXmppStarted)
        Sleep(1000 / 30);

    std::string strPort;
    std::cout << _("Enter the port for the main server: ");
    std::getline(std::cin, strPort);
    int Port = std::stoi(strPort);

    svr.Get("/", [](const Request& req, Response& res) {
        res.set_content(_("Argon Backend"), _("text/plain"));
        });
	
    std::thread BackendThread = std::thread(StartBackend, Port);
	
    std::cout << std::format(_("Listening on port: {}!\n"), Port);
    SetConsoleTitleA((_("Argon Backend | Port ") + std::to_string(Port)).c_str());
}
