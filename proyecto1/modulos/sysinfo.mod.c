#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x367fcc51, "module_layout" },
	{ 0xc4bcd79c, "seq_read" },
	{ 0x8d5ebc6b, "remove_proc_entry" },
	{ 0x92997ed8, "_printk" },
	{ 0xaaf8f76a, "proc_create" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x37a0cba, "kfree" },
	{ 0x489f1f27, "mmput" },
	{ 0x93466e95, "access_process_vm" },
	{ 0x53b954a2, "up_read" },
	{ 0x668b19a1, "down_read" },
	{ 0x34fa9190, "get_task_mm" },
	{ 0xcbb0ae81, "kmem_cache_alloc_trace" },
	{ 0x58f94a7a, "kmalloc_caches" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x6e50190f, "init_task" },
	{ 0xb20dd93f, "seq_printf" },
	{ 0x40c7247c, "si_meminfo" },
	{ 0x15ba50a6, "jiffies" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xc22aaf3c, "single_open" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "2D9A2D2AAD1D60DE21ADF8F");
