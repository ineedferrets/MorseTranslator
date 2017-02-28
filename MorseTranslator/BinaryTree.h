#pragma once

struct node {
  // character
  char key_value;
  // dot
  node *left;
  // dash
  node *right;

  node(char key) {
    key_value = key;
    left = 0;
    right = 0;
  };
};

class BinaryTree
{
public:
  BinaryTree();
  BinaryTree(node *rootNode);
  ~BinaryTree();

  node *getRoot();
  void destroyTree();

private:
  void destroyTree(node *leaf);

  node *root;
};

