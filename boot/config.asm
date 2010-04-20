%define stage1 0x7c00
%define stage2 0x9000
%define buffer 0x10000
; kernel will be loaded after 64k from buffer (64k is biggest possible cluster in fat32)
%define kernel buffer+0x10000
%define stack 0x9000
%define FAT_Cluster_Mask 0x0fffffff
%define FAT_EOF 0x0ffffff8