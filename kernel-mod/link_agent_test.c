#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <net/genetlink.h>


#include "link_agent_test.h"
#include "netlink.h"


// log macro
#ifdef pr_fmt
#undef pr_fmt
#endif

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt



static int __init link_agent_mod_init(void) {
    pr_info("Link agent started");
    int ret;
    // init generic netlink
    ret = agent_genetlink_init();
    if (ret < 0) {
        goto err_genetlink;
    }

    return 0;

err_genetlink:
    agent_genetlink_uninit();
    return ret;
}


static void __exit link_agent_mod_exit(void) {
    pr_info("Link agent ended");
    agent_genetlink_uninit();
    return;
}

module_init(link_agent_mod_init);
module_exit(link_agent_mod_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("zhouxiaoming <zhouxiaoming@datacloak.com>");
MODULE_DESCRIPTION(
    "linux module that registers the custom netlink family"
);
MODULE_VERSION("1.0.0");















