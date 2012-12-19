#include "linked_structures.h"
static bstree*
new_bsnode(void* data, BOOLEAN deep_copy, list_tspec*type){
	if(root == NULL) return;
	print_splay_values(root->left);
	print_splay_values(root->right);
	printf("<%p,%p,%p> %p\n", root->left, root, root->right, root->data);
}

bstree*
bstree_insert(bstree *root, void* key, BOOLEAN copy, list_tspec* type){
	bstree n = bstree_find(root, key, type);
	//TODO
}

bstree*
bstree_find(bstree *root, bstree *key, list_tspec* type){
	long c;
	if(root == NULL) return root;
	while((c = type->compar(key, root->data)) != 0){
		if(c < 0){
			if(root->left == NULL) break;
			root = root->left;
		} else {
			if(root->right == NULL) break;
			root = root->right;
		}
	}
	return root;
}

bstree*
bstree_parent(bstree *root, bstree *node, list_tspec* type){
	long c;
	bstree *parent = root;
	if(root == NULL || node == NULL) return root;
	while((c = type->compar(node->data, root->data)) != 0){
		parent = root;
		if(c < 0){
			if(root->left == NULL) break;
			root = root->left;
		} else {
			if(root->right == NULL) break;
			root = root->right;
		}
	}
	return parent;
}

dlist* /* with type bstree*/
bstree_path(bstree *root, bstree *node, list_tspec* type){
	dlist *head = NULL;
	long c;
	if(root == NULL || node == NULL) return root;
	while((c = type->compar(node->data, root->data)) != 0){
		head = dlist_append(head, root, false, type);
		if(c < 0){
			if(root->left == NULL) break;
			root = root->left;
		} else {
			if(root->right == NULL) break;
			root = root->right;
		}
	}
	return dlist_append(head, root, false, type);
}

bstree*
bstree_predessor(bstree* root, bstree* node, list_tspec* type){
	if(root == NULL || node == NULL) return NULL;
	if( node->left != NULL ) return bstree_findmax(node->left);
	bstree* parent = bstree_parent(root, node, type);//use bstree_path instead
	while(parent->left == node && parent != root){
		node = parent;
		parent = bstree_parent(root, node, type);
	}
}

bstree*
bstree_findmin(bstree* root){
	if(root == NULL) return root;
	while(root->left != NULL) root = root->left;
	return root;
}

bstree*
bstree_findmax(bstree* root){
	if(root == NULL) return root;
	while(root->right != NULL) root = root->right;
	return root;
}
