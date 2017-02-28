#include "BinaryTree.h"

BinaryTree::BinaryTree() {
	root = 0;
}

BinaryTree::BinaryTree(node *rootNode) {
	root = rootNode;
}

BinaryTree::~BinaryTree() {
}

node *BinaryTree::getRoot() {
	return root;
}

void BinaryTree::destroyTree() {
	destroyTree(root);
}

void BinaryTree::destroyTree(node *leaf) {
	if (leaf != 0) {
		destroyTree(leaf->left);
		destroyTree(leaf->right);
		delete leaf;
	}
}