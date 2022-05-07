// made by me, was pain and bad code and slow and alot of problems but idc

#include <vector>

class XML
{
private:
    std::string data;
    std::string name;
    bool m_HasClosingTag = false;
    std::vector<XML*> nodes;
    XML* Owner = nullptr;

    void Update()
    {
        if (nodes.size() > 0)
        {
            for (int i = 0; i < nodes.size(); i++)
            {
                auto currentNode = nodes[i];
                auto childNodes = currentNode->GetNodes();
                auto nodeStr = currentNode->Get();
                // if (m_HasClosingTag) data.insert(data.size() - 3 - name.size(), nodeStr);
                // else data.insert(data.size() - 3, nodeStr);

                // Adding Nodes

                for (int i = 0; i < childNodes.size(); i++)
                {
                    auto childNode = childNodes[i];
                    if (i < childNodes.size())
                    {
                        auto nodeStrr = childNode->Get();
                        if (m_HasClosingTag) data.insert((data.size() - 3 - name.size()) - currentNode->GetName().size() - 3, nodeStrr);
                        else data.insert((data.size() - 3) - 3, nodeStrr);
                    }
                }

            }
        }
    }
public:
    XML AddAttribute(const std::string& attr, const std::string& val)
    {
        std::string Final = " " + attr + "=\"" + val + "\"";
        if (m_HasClosingTag) data.insert(data.size() - 4 - name.size(), Final);
        else data.insert(data.size() - 2, Final);
        if (Owner)
        {
            int insertAt = 0;
            auto OwnerNodes = Owner->GetNodes();
            for (int i = 0; i < OwnerNodes.size(); i++) // Calcuate the point to insert at
            {
                insertAt += OwnerNodes.at(i)->GetName().size();
            }
            if (Owner->m_HasClosingTag) Owner->data.insert(Owner->data.size() - insertAt - (Owner->GetName().size() + 2) - (OwnerNodes.size() * 4), Final);
            else Owner->data.insert(Owner->data.size() - insertAt - 3, Final);
        }
        return *this;
    }

    XML SetValue(const std::string& val)
    {
        if (m_HasClosingTag) data.insert(data.size() - 3 - name.size(), val);
        else data.insert(data.size() - 2, val);
        if (Owner)
        {
            int insertAt = 0;
            auto OwnerNodes = Owner->GetNodes();
            for (int i = 0; i < OwnerNodes.size(); i++) // Calcuate the point to insert at
            {
                auto node = OwnerNodes.at(i);
                insertAt += node->GetName().size();
            }
            if (Owner->m_HasClosingTag) Owner->data.insert(Owner->data.size() - insertAt - (Owner->GetName().size() + 2) - (OwnerNodes.size() * 4), val);
            else Owner->data.insert(Owner->data.size() - insertAt - 2, val);
        }
        return *this;
    }

    XML* AddNode(const std::string& node, bool bClosingTag = true)
    {
        XML* Node = new XML(node, bClosingTag);
        Node->Owner = this;
        nodes.push_back(Node);
        std::string nodeStr = Node->Get();
        // std::cout << nodeStr << '\n';
        if (m_HasClosingTag) data.insert(data.size() - 3 - name.size(), nodeStr);
        else data.insert(data.size() - 2, nodeStr);
        return Node;
    }

    std::vector<XML*> GetNodes() const { return nodes; }
    std::string GetName() const { return name; }
    XML* GetNode(int Index) const { return nodes[Index]; }

    XML* operator[](int i) { return nodes.at(i); }

    XML() { data = "</>"; }
    XML(const std::string& start) { name = start; data = "<" + start + "/>"; }
    XML(const std::string& start, bool bClosingTag)
    {
        m_HasClosingTag = bClosingTag;
        if (!bClosingTag) XML(start);
        name = start;
        data = "<" + start + "></" + start + ">";
    }
    std::string Get()
    {
        Update();
        return data;
    }
};