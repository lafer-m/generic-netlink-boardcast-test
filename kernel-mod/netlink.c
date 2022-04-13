

#include "netlink.h"
#include "link_agent_test.h"
#include <net/genetlink.h>
#include <net/sock.h>
#include <linux/timer.h>




// init a kernel timer
static struct timer_list my_timer;

static const struct nla_policy agent_policy[GNL_AGENT_ATTRIBUTE_COUNT+1] = {
    [GNL_AGENT_A_UNSPEC] = {.type = NLA_UNSPEC},
    [GNL_AGENT_A_MSG]  = {.type = NLA_NUL_STRING}
};

static struct genl_multicast_group groups[] = {
    {   .name = AGENT_GENL_GROUP },
};

int genl_doit_reply_with_nlmsg_err(struct sk_buff *sender, struct genl_info *info) {
    pr_info("%s() invoked, a err will be send back\n", __func__);
    return -EINVAL;
}

struct genl_ops genl_ops[] = {
    {
        .cmd = GNL_AGENT_C_ECHO_MSG,
        .policy = agent_policy,
        .doit = genl_doit_reply_with_nlmsg_err,
    },
    {
        .cmd = GNL_AGENT_C_REGISTER,
        .policy = agent_policy,
        .doit = genl_doit_reply_with_nlmsg_err,
    }
};

static struct genl_family genl_family
__ro_after_init = {
    .ops = genl_ops,
    .n_ops = ARRAY_SIZE(genl_ops),
    .mcgrps = groups,
    .n_mcgrps = 1,
    .name = AGENT_GENL_NAME,
    .version = AGENT_GENL_VERSION,
    .maxattr = GNL_AGENT_ATTRIBUTE_COUNT,
    .module = THIS_MODULE,
    // .policy = agent_policy,

};


static void timer_callback(struct timer_list *timer) {
    pr_info("timer callback start\n");
    struct sk_buff *send_skb;
    void *msg_head;
    char *send_str = "hello timer test!";
    int ret;

    send_skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
    if (send_skb == NULL) {
        pr_err("genlmsg new err happened\n");
        goto fast_return;
    }
    msg_head = genlmsg_put(send_skb,0,0, &genl_family, 0, GNL_AGENT_C_REGISTER);
    if (msg_head == NULL) {
        pr_err("genlmsg pue err happened\n");
        // nlmsg_free(send_skb);
        goto fast_return;
    }

    ret = nla_put_string(send_skb,GNL_AGENT_A_MSG,send_str);
    if (ret != 0) {
        pr_err("nla put string err\n");
        // nlmsg_free(send_skb);
        goto fast_return;
    }

    genlmsg_end(send_skb, msg_head);

    ret = genlmsg_multicast_allns(&genl_family,send_skb, 0, 0, GFP_KERNEL);
    if (ret != 0) {
        pr_err("nla multicast err: %i\n", ret);
        goto fast_return;
    }

fast_return:
    // set the timer interval to 1000 msecs
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000));
}


int __init agent_genetlink_init(void) 
{
    int ret;
    ret = genl_register_family(&genl_family);
    if (ret < 0) {
        genl_unregister_family(&genl_family);
        return ret;
    }
    // setup the timer
    timer_setup(&my_timer, timer_callback, 0);
    // set the timer interval to 1000 msecs
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000));

    return 0;
}   

void __exit agent_genetlink_uninit(void) 
{
    genl_unregister_family(&genl_family);
    del_timer(&my_timer);
    return;
}





