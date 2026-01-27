#include <linux/cache.h>
#include <linux/memcontrol.h>
#include <linux/module.h>
#include <linux/numa.h>
#include <linux/notifier.h>
#include <linux/printk.h>
#include <linux/htmm.h>
#include <linux/sched.h>

struct htmm_migration_timer_record {
    u64 time;
    struct mem_cgroup_per_node *memcg_pn;
} ____cacheline_aligned;

const char *MIGRATION_TYPE_TEXTS[2] = {"promotion", "demotion"};
struct htmm_migration_timer_record *mig_timer_records;

static int migration_notifier_cb(struct notifier_block *nb, unsigned long action, void *data) {
    union htmm_notifier_data *ndata = (union htmm_notifier_data *)data;
    char *migration_type;
    switch (action) {
        int node_id;
        case HTMM_PROMOTE_START:
        case HTMM_DEMOTE_START:
            node_id = ndata->migrate_info.node_id;
            mig_timer_records[node_id].memcg_pn = ndata->migrate_info.memcg_pn;
            mig_timer_records[node_id].time = ktime_get_ns();
        case HTMM_PROMOTE_END: 
        case HTMM_DEMOTE_END:
            node_id = ndata->migrate_info.node_id;
            unsigned long prev_time = mig_timer_records[node_id].time;
            unsigned long curr_time = ktime_get_ns();
            struct mem_cgroup_per_node *prev_memcg_pn = mig_timer_records[node_id].memcg_pn;
            struct mem_cgroup_per_node *curr_memcg_pn = ndata->migrate_info.memcg_pn;
            if (action == HTMM_PROMOTE_END)
                migration_type = MIGRATION_TYPE_TEXTS[1];
            else
                migration_type = MIGRATION_TYPE_TEXTS[0];

            if (prev_memcg_pn == curr_memcg_pn)
                pr_info("memtis-migratemon: %s time %llu ns for mem cgroup %s at node %d \n", migration_type, curr_time - prev_time, curr_memcg_pn->memcg->css.cgroup, node_id);
            mig_timer_records[node_id].memcg_pn = curr_memcg_pn;
            mig_timer_records[node_id].time = curr_time;
            break;
        default:
            break;
    }
    return NOTIFY_OK;
}

static struct notifier_block my_nb = {
        .notifier_call = migration_notifier_cb,
    };

int init_module(void) {
    int nid;
    int max_node_id = 0;

    for_each_node_state(nid, N_MEMORY) {
        if (nid > max_node_id) {
            max_node_id = nid;
        }
    }

    pr_info("memtis-migratemon: Running with max_node_id = %d", max_node_id);
    mig_timer_records = kmalloc(sizeof(*mig_timer_records) * (max_node_id + 1), GFP_KERNEL);
    if (mig_timer_records) {
        register_htmm_notifier(&my_nb); 
    }
    return 0;
}

void cleanup_module(void) {
    pr_info("memtis-migratemon: Stopped.\n");
    if (!mig_timer_records) 
        return;
    unregister_htmm_notifier(&my_nb);
    kfree(mig_timer_records);
}

MODULE_LICENSE("GPL");
