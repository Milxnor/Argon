/*

NOTICE:

This code is extremely old, presence is the only working thing. And this code is really bad but I'll just leave it here for now.

*/

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_

#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <rapidxml/rapidxml.hpp>
#include <json.hpp>

#include <uuid.h>

#include <iostream>
#include <Windows.h>
#include <vector>
#include <format>

#include "xml.h"
#include <xorstr.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using namespace rapidxml;
using namespace uuids;

static std::string domain = "Argon.Server";

typedef server::message_ptr message_ptr;

static bool bHasXmppStarted = false;

std::string GenerateJID(const std::string& Name, const std::string& Resource, const std::string& Domain = domain)
{
    return std::format("{}@{}/{}", Name, Resource, Domain);
}

class Client
{
private:
    std::string m_Username = "";
    std::string m_Token = "";
    std::string m_Resource = "";
    std::string m_Jid = "";
    uuid m_UUID;

    uuid SetupUUID()
    {
        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size> {};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937 generator(seq);

        m_UUID = uuid_random_generator{ generator }();
    }

public:
    void Init(const std::string& JID, const std::string& token)
    {
        m_Jid = JID;
        m_Token = token;
        // name@domain/resource
        // TODO: Split JID to find name and resource

        SetupUUID();
    }

    void Init(const std::string& name, const std::string& resource, const std::string& token)
    {
        m_Username = name;
        m_Resource = resource;
        m_Token = token;
        m_Jid = GenerateJID(name, resource, token);

        SetupUUID();
    }

    bool IsSetup() {
        if (m_Username == "" || m_Token == "" || m_Resource == "" || m_Jid == "") return false;
        return true;
    }

    const std::string GetJID() const { return m_Jid; }
    const inline std::string GetResource() const { return m_Resource; }
    const inline std::string GetName() const { return m_Username; }
};

static std::vector<Client*> Clients;

Client* GetClientByName(const std::string& name)
{
    for (int i = 0; i < Clients.size(); i++)
    {
        if (Clients[i]->GetName() == name) return Clients[i];
    }
    return nullptr;
}

namespace Handlers
{
    void Message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
        std::string load = msg->get_payload();
        xml_document<> doc;
        // std::cout << "Message received: " << load << '\n';
        doc.parse<0>((char*)load.c_str());
        std::string feature = doc.first_node()->name();
        if (feature == _("open"))
        {
            std::cout << _("New connection!\n");
            XML Xml(_("open"));
            Xml.AddAttribute(_("id"), _("2"));
            Xml.AddAttribute(_("xmlns"), _("urn:ietf:params:xml:ns:xmpp-framing"));
            Xml.AddAttribute(_("from"), domain);
            s->send(hdl, Xml.Get(), websocketpp::frame::opcode::value::TEXT);
        }
        else if (feature == _("iq")) // Used for Non-SASL Authentication
        {
            std::string id = doc.first_node()->first_attribute("id")->value();
            if (id == _("_xmpp_auth1"))
            {
                std::string username = doc.first_node()->first_node()->first_node()->value();
                std::string oauth = doc.first_node()->first_node()->first_node()->next_sibling()->value();
                std::string resource = doc.first_node()->first_node()->first_node()->next_sibling()->next_sibling()->value();

                auto CurrentClient = GetClientByName(username);
                if (!CurrentClient)
                {
                    Client* newClient = new Client;
                    newClient->Init(username, oauth, resource);
                    Clients.push_back(newClient);
                    CurrentClient = newClient;
                }

                std::cout << username << _(" tried to join with token ") << oauth << "!\n";
                std::cout << _("Resource: ") << resource << '\n';
                XML Xml(_("iq"), true);
                Xml.AddAttribute(_("type"), _("result"));
                Xml.AddAttribute(_("id"), id);
                Xml.AddAttribute(_("xmlns"), "jabber:client");
                Xml.SetValue(_("Authentication sucasdazawcessful."));
                s->send(hdl, Xml.Get(), websocketpp::frame::opcode::value::TEXT);
            }
            else if (id == _("_xmpp_bind1"))
            {
                std::cout << _("XMPP Bind Received.\n");
            }
            else if (std::string(doc.first_node()->first_node()->name()).find(_("ping")) == std::string::npos)
                std::cout << _("Invalid Id! ") << id << '\n';
            // std::cout << doc.first_node()->first_node()->name() << '\n';
        }
        else if (feature == _("presence"))
        {
            std::string presencejson(doc.first_node()->first_node()->value());
            // presencejson.resize(presencejson.size() - 4);
            std::string DisplayName = nlohmann::json::parse(presencejson)[_("Properties")][_("party.joininfodata.286331153_j")][_("sourceDisplayName")]; // TODO: Like not do this
            auto client = GetClientByName(DisplayName);

        }
        else if (feature == _("close"))
        {
            std::cout << _("Closing connection.\n");
            // TODO: Remove from all Connections
        }
        else
            std::cout << _("Unknown feature! ") << feature << '\n';

        std::cout << load << '\n';

        // s->send(hdl, msg->get_payload(), msg->get_opcode()); // Echo the message back.
    }
}

int StartXMPP(int xmppPort = 80) {

    server svr;

    try {
        svr.set_access_channels(websocketpp::log::alevel::all);
        svr.clear_access_channels(websocketpp::log::alevel::frame_payload);

        svr.init_asio();

        svr.set_message_handler(bind(&Handlers::Message, &svr, ::_1, ::_2));

        svr.listen(xmppPort);

        svr.start_accept();

        std::cout << _("Started running XMPP on port ") << xmppPort << ".\n";
        // SetConsoleTitleA(("XMPP Server | Port " + std::to_string(xmppPort)).c_str());

        bHasXmppStarted = true;
        
        svr.run();
    }
    catch (websocketpp::exception const& e) {
        std::cout << e.what() << std::endl;
    }
    catch (...) {
        std::cout << _("other exception") << std::endl;
    }
}