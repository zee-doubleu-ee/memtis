#include <linux/cache.h>
#include <linux/gfp.h>
#include <linux/memcontrol.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/nodemask.h>
#include <linux/numa.h>
#include <linux/perf_event.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/sysfs.h>
#include <linux/htmm.h>

struct htmm_migration_timer_record {
    u64 time;
    struct mem_cgroup_per_node *memcg_pn;
};

struct htmm_kmigraterd_info {
    int node_id;
    struct kobject kobj;
    struct task_struct *task;
    struct perf_event *cycle_counter;
    struct htmm_migration_timer_record mig_timer_record;
} ____cacheline_aligned;

#define kmigraterd_info_of(kobj) container_of(kobj, struct htmm_kmigraterd_info, kobj)

static struct htmm_kmigraterd_info *kmigraterd_infos;

#define KMIGRATERD_SCHED_STAT_SHOW(name) \
static ssize_t name##_show(struct kobject *kobj, \
    struct kobj_attribute *attr, char *buf) \
{ \
    u64 value = 0; \
    struct task_struct *kmigraterd = READ_ONCE(kmigraterd_info_of(kobj)->task); \
    if (kmigraterd) { \
        value = kmigraterd->se.statistics.name; \
    } \
    return sysfs_emit(buf, "%llu\n", value); \
} \
static struct kobj_attribute dev_attr_kmigraterd_##name = __ATTR_RO(name);

KMIGRATERD_SCHED_STAT_SHOW(wait_max)
KMIGRATERD_SCHED_STAT_SHOW(wait_count)
KMIGRATERD_SCHED_STAT_SHOW(wait_sum)
KMIGRATERD_SCHED_STAT_SHOW(iowait_count)
KMIGRATERD_SCHED_STAT_SHOW(iowait_sum)
KMIGRATERD_SCHED_STAT_SHOW(sleep_max)
KMIGRATERD_SCHED_STAT_SHOW(sum_sleep_runtime)
KMIGRATERD_SCHED_STAT_SHOW(block_max)
KMIGRATERD_SCHED_STAT_SHOW(exec_max)
KMIGRATERD_SCHED_STAT_SHOW(slice_max)
KMIGRATERD_SCHED_STAT_SHOW(nr_migrations_cold)
KMIGRATERD_SCHED_STAT_SHOW(nr_failed_migrations_affine)
KMIGRATERD_SCHED_STAT_SHOW(nr_failed_migrations_running)
KMIGRATERD_SCHED_STAT_SHOW(nr_failed_migrations_hot)
KMIGRATERD_SCHED_STAT_SHOW(nr_forced_migrations)
KMIGRATERD_SCHED_STAT_SHOW(nr_wakeups)
KMIGRATERD_SCHED_STAT_SHOW(nr_wakeups_sync)
KMIGRATERD_SCHED_STAT_SHOW(nr_wakeups_migrate)
KMIGRATERD_SCHED_STAT_SHOW(nr_wakeups_local)
KMIGRATERD_SCHED_STAT_SHOW(nr_wakeups_remote)
KMIGRATERD_SCHED_STAT_SHOW(nr_wakeups_affine)
KMIGRATERD_SCHED_STAT_SHOW(nr_wakeups_affine_attempts)
KMIGRATERD_SCHED_STAT_SHOW(nr_wakeups_passive)
KMIGRATERD_SCHED_STAT_SHOW(nr_wakeups_idle)

static ssize_t cycles_show(struct kobject *kobj,
    struct kobj_attribute *attr, char *buf)
{
	u64 enabled, running, cycles = 0;
	struct htmm_kmigraterd_info *info = kmigraterd_info_of(kobj);
	if (info->cycle_counter)
		cycles = perf_event_read_value(info->cycle_counter, &enabled, &running);
	return sysfs_emit(buf, "%llu\n", cycles);
}
static struct kobj_attribute dev_attr_kmigraterd_cycles = __ATTR_RO(cycles);

static ssize_t sum_exec_runtime_show(struct kobject *kobj,
    struct kobj_attribute *attr, char *buf)
{
    u64 value = 0;
    struct task_struct *kmigraterd = READ_ONCE(kmigraterd_info_of(kobj)->task);
    if (kmigraterd) {
        value = kmigraterd->se.sum_exec_runtime;
    }
    return sysfs_emit(buf, "%llu\n", value);
}

static struct kobj_attribute dev_attr_kmigraterd_sum_exec_runtime = __ATTR_RO(sum_exec_runtime);

static ssize_t nr_migrations_show(struct kobject *kobj,
    struct kobj_attribute *attr, char *buf)
{
    u64 value = 0;
    struct task_struct *kmigraterd = READ_ONCE(kmigraterd_info_of(kobj)->task);
    if (kmigraterd) {
        value = kmigraterd->se.nr_migrations;
    }
    return sysfs_emit(buf, "%llu\n", value);
}

static struct kobj_attribute dev_attr_kmigraterd_nr_migrations = __ATTR_RO(nr_migrations);

static struct attribute *kmigraterd_attrs[] = {
    &dev_attr_kmigraterd_wait_max.attr,
    &dev_attr_kmigraterd_wait_count.attr,
    &dev_attr_kmigraterd_wait_sum.attr,
    &dev_attr_kmigraterd_iowait_count.attr,
    &dev_attr_kmigraterd_iowait_sum.attr,
    &dev_attr_kmigraterd_sleep_max.attr,
    &dev_attr_kmigraterd_sum_sleep_runtime.attr,
    &dev_attr_kmigraterd_block_max.attr,
    &dev_attr_kmigraterd_exec_max.attr,
    &dev_attr_kmigraterd_slice_max.attr,
    &dev_attr_kmigraterd_nr_migrations_cold.attr,
    &dev_attr_kmigraterd_nr_failed_migrations_affine.attr,
    &dev_attr_kmigraterd_nr_failed_migrations_running.attr,
    &dev_attr_kmigraterd_nr_failed_migrations_hot.attr,
    &dev_attr_kmigraterd_nr_forced_migrations.attr,
    &dev_attr_kmigraterd_nr_wakeups.attr,
    &dev_attr_kmigraterd_nr_wakeups_sync.attr,
    &dev_attr_kmigraterd_nr_wakeups_migrate.attr,
    &dev_attr_kmigraterd_nr_wakeups_local.attr,
    &dev_attr_kmigraterd_nr_wakeups_remote.attr,
    &dev_attr_kmigraterd_nr_wakeups_affine.attr,
    &dev_attr_kmigraterd_nr_wakeups_affine_attempts.attr,
    &dev_attr_kmigraterd_nr_wakeups_passive.attr,
    &dev_attr_kmigraterd_nr_wakeups_idle.attr,

    &dev_attr_kmigraterd_cycles.attr,
    &dev_attr_kmigraterd_sum_exec_runtime.attr,
    &dev_attr_kmigraterd_nr_migrations.attr,
    NULL,
};

static const struct attribute_group kmigraterd_attr_group = {
    .attrs = kmigraterd_attrs,
};

static const struct attribute_group *kmigraterd_attr_groups[] = {
    &kmigraterd_attr_group,
    NULL,
};

void release_kmigraterd_info(struct kobject *kobj) {
    struct htmm_kmigraterd_info *info = kmigraterd_info_of(kobj);
    if (info->cycle_counter)
        perf_event_release_kernel(info->cycle_counter);
}

static struct kobj_type htmm_kmigraterd_info_ktype = {
    .release = release_kmigraterd_info,
    .sysfs_ops = &kobj_sysfs_ops,
    .default_groups = kmigraterd_attr_groups,
};

void init_kmigraterd_info(struct htmm_kmigraterd_info *info, struct task_struct *task) {
    struct perf_event_attr attr = {
		.type = PERF_TYPE_HARDWARE,
		.size = sizeof(struct perf_event_attr),
		.config = PERF_COUNT_HW_CPU_CYCLES,
		.disabled = 0,
		.exclude_kernel = 0,
		.exclude_user = 1,
		.exclude_idle = 1,
		.exclude_hv = 1,
	};
    if (info->cycle_counter) {
        perf_event_release_kernel(info->cycle_counter);
        info->cycle_counter = NULL;
    }

    info->node_id = (info - kmigraterd_infos);
    WRITE_ONCE(info->task, task);
    if (info->kobj.state_initialized)
        kobject_put(&info->kobj);
    if (!task)
        return;
    kobject_init_and_add(&info->kobj, &htmm_kmigraterd_info_ktype, htmm_kobj, "kmigraterd%d", info->node_id);
    WRITE_ONCE(info->cycle_counter, perf_event_create_kernel_counter(&attr, -1, task, NULL, NULL));
}

const char *MIGRATION_TYPE_TEXTS[2] = {"promotion", "demotion"};

static int migration_notifier_cb(struct notifier_block *nb, unsigned long action, void *data) {
    union htmm_notifier_data *ndata = (union htmm_notifier_data *)data;
    switch (action) {
        int node_id;
        case HTMM_PROMOTE_START:
        case HTMM_DEMOTE_START:
            node_id = ndata->migrate_info.node_id;
            if (unlikely(kmigraterd_infos[node_id].task != current)) {
                init_kmigraterd_info(&kmigraterd_infos[node_id], current);
            }
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
    int nid, max_nid = 0;

    if (!htmm_kobj) 
        return -ENODEV;

    for_each_node_state(nid, N_MEMORY) {
        if (nid > max_nid)
            max_nid = nid;
    }
    
    kmigraterd_infos = kzalloc(sizeof(struct htmm_kmigraterd_info) * (max_nid + 1), GFP_KERNEL);
    if (!kmigraterd_infos)
        return -ENOMEM;

    for_each_node_state(nid, N_MEMORY)
        init_kmigraterd_info(&kmigraterd_infos[nid], NULL);

    pr_info("memtis-migratemon: Running with max_nid = %d\n", max_nid);
    register_htmm_notifier(&my_nb); 
    return 0;
}

void cleanup_module(void) {
    int nid;
    pr_info("memtis-migratemon: Stopped.\n");

    if (kmigraterd_infos) {
        for_each_node_state(nid, N_MEMORY) {
            if (kmigraterd_infos[nid].kobj.state_initialized)
                kobject_put(&kmigraterd_infos[nid].kobj);
        }
        kfree(kmigraterd_infos);
    }

    unregister_htmm_notifier(&my_nb);
}

MODULE_LICENSE("GPL");
