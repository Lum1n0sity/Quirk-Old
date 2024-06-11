#ifndef ASTNODE_H
#define ASTNODE_H

#include <string>
#include <vector>

class ASTNode {
public:
    ASTNode(std::string type, std::string value = "", bool processed_ = false);

    void add_child(ASTNode* node);
    void set_parent(ASTNode* parent);
    void set_type(const std::string& newType);
    void set_value(const std::string& newValue);

    ASTNode* get_parent() const;
    std::string getType() const;
    std::string getValue() const;
    const std::vector<ASTNode*>& getChildren() const;

    bool isProcessed() const { return processed_; }
    void setProcessed(bool processed) { processed_ = processed; }
    
private:
    std::string type;
    std::string value;
    std::vector<ASTNode*> children;
    ASTNode* parent_;
    bool processed_;
};

#endif
