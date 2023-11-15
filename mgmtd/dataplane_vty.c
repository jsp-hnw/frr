#include <zebra.h>

#include "command.h"
#include "northbound_cli.h"

#include "mgmtd/dataplane_vty_clippy.c"

#define VLB_STR "Balancer configuration\n"
#define BALANCER_ALGS_STR "Source IP hash algorithm\nWeighted Least-Connection algorithm\nWeighted Round Robin algorithm\n"

static inline const char *
algoithm_to_enum(const char *algorithm)
{
	if (strmatch(algorithm, "source-hash")) {
		return "sh";
	} else {
		return algorithm;
	}
}

DEFPY_YANG (
	vtysh_mgt_target_group,
	vtysh_mgt_target_group_cmd,
	"vlb target-group NAME algorithm <source-hash|wlc|wrr>$algorithm",
	VLB_STR
	"Target-group configuration\n"
	"The name of the target group\n"
	"Load balancing algorithm\n"
	BALANCER_ALGS_STR)
{
	nb_cli_enqueue_change(vty, ".", NB_OP_MODIFY, NULL);
	nb_cli_enqueue_change(vty, "./algorithm", NB_OP_MODIFY, algoithm_to_enum(algorithm));
	return nb_cli_apply_changes(vty, "/nfware-adc:adc/target-groups/target-group[name='%s']", target_group);
}

DEFPY_YANG (
	vtysh_mgt_no_target_group,
	vtysh_mgt_no_target_group_cmd,
	"no vlb target-group NAME [algorithm <source-hash|wlc|wrr>]",
	NO_STR
	VLB_STR
	"Target-group configuration\n"
	"The name of the target group\n"
	"Load balancing algorithm\n"
	BALANCER_ALGS_STR)
{
	nb_cli_enqueue_change(vty, ".", NB_OP_DESTROY, NULL);
	return nb_cli_apply_changes(vty, "/nfware-adc:adc/target-groups/target-group[name='%s']", target_group);
}

static int
vlb_target_group_cli_cmp(const struct lyd_node *dnode1, const struct lyd_node *dnode2)
{
	const char *name1 = yang_dnode_get_string(dnode1, "name");
	const char *name2 = yang_dnode_get_string(dnode2, "name");

	return strcmp(name1, name2);
}

static void
vlb_target_group_cli_show(struct vty *vty, const struct lyd_node *dnode, bool show_defaults)
{
	const char *name = yang_dnode_get_string(dnode, "name");
	const char *algorithm = yang_dnode_get_string(dnode, "algorithm");
	const char *output;

	if (strmatch(algorithm, "sh")) {
		output = "source-hash";
	} else {
		output = algorithm;
	}

	vty_out(vty, "vlb target-group %s algorithm %s\n", name, output);
}

DEFPY_YANG (
	vtysh_mgt_target_group_worker,
	vtysh_mgt_target_group_worker_cmd,
	"vlb target-group NAME worker ip A.B.C.D port (0-65535) [{weight (0-100000000) | health-monitor NAME | nat-pool NAME}]",
	VLB_STR
	"Target-group configuration\n"
	"The name of the target group\n"
	"Worker configuration\n"
	"IPv4 address of the worker\n"
	"IPv4 address of the worker\n"
	"L4 port of the worker\n"
	"L4 port of the worker\n"
	"Weight of the worker for load balancing\n"
	"Weight of the worker for load balancing\n"
	"Health-monitor for the worker\n"
	"Health-monitor name\n"
	"Source NAT pool for the worker\n"
	"Source NAT pool name\n")
{
	nb_cli_enqueue_change(vty, ".", NB_OP_MODIFY, NULL);
	if (weight_str) {
		nb_cli_enqueue_change(vty, "./weight", NB_OP_MODIFY, weight_str);
	} else {
		nb_cli_enqueue_change(vty, "./weight", NB_OP_DESTROY, NULL);
	}
	if (health_monitor) {
		nb_cli_enqueue_change(vty, "./health-monitor", NB_OP_MODIFY, health_monitor);
	} else {
		nb_cli_enqueue_change(vty, "./health-monitor", NB_OP_DESTROY, NULL);
	}
	if (nat_pool) {
		nb_cli_enqueue_change(vty, "./pool-name", NB_OP_MODIFY, nat_pool);
	} else {
		nb_cli_enqueue_change(vty, "./pool-name", NB_OP_DESTROY, NULL);
	}
	return nb_cli_apply_changes(vty, "/nfware-adc:adc/target-groups/target-group[name='%s']/workers/worker[ip='%s'][port='%s']", target_group, ip_str, port_str);
}

DEFPY_YANG (
	vtysh_mgt_no_target_group_worker,
	vtysh_mgt_no_target_group_worker_cmd,
	"no vlb target-group NAME worker ip A.B.C.D port (0-65535) [{weight (0-100000000) | health-monitor NAME | nat-pool NAME}]",
	NO_STR
	VLB_STR
	"Target-group configuration\n"
	"The name of the target group\n"
	"Worker configuration\n"
	"IPv4 address of the worker\n"
	"IPv4 address of the worker\n"
	"L4 port of the worker\n"
	"L4 port of the worker\n"
	"Weight of the worker for load balancing\n"
	"Weight of the worker for load balancing\n"
	"Health-monitor for the worker\n"
	"Health-monitor name\n"
	"Source NAT pool for the worker\n"
	"Source NAT pool name\n")
{
	nb_cli_enqueue_change(vty, ".", NB_OP_DESTROY, NULL);
	return nb_cli_apply_changes(vty, "/nfware-adc:adc/target-groups/target-group[name='%s']/workers/worker[ip='%s'][port='%s']", target_group, ip_str, port_str);
}

static int
vlb_target_group_worker_cli_cmp(const struct lyd_node *dnode1, const struct lyd_node *dnode2)
{
	struct prefix prefix1, prefix2;
	uint16_t port1, port2;
	int ret;

	yang_dnode_get_prefix(&prefix1, dnode1, "ip");
	yang_dnode_get_prefix(&prefix2, dnode2, "ip");
	ret = prefix_cmp(&prefix1, &prefix2);
	if (ret)
		return ret;

	port1 = yang_dnode_get_uint16(dnode1, "port");
	port2 = yang_dnode_get_uint16(dnode2, "port");

	return (int)port1 - (int)port2;
}

static void
vlb_target_group_worker_cli_show(struct vty *vty, const struct lyd_node *dnode, bool show_defaults)
{
	const char *name = yang_dnode_get_string(dnode, "../../name");
	const char *ip = yang_dnode_get_string(dnode, "ip");
	const char *port = yang_dnode_get_string(dnode, "port");

	vty_out(vty, "vlb target-group %s worker ip %s port %s", name, ip, port);

	if (yang_dnode_exists(dnode, "weight"))
		vty_out(vty, " weight %s", yang_dnode_get_string(dnode, "weight"));
	if (yang_dnode_exists(dnode, "health-monitor"))
		vty_out(vty, " health-monitor %s", yang_dnode_get_string(dnode, "health-monitor"));
	if (yang_dnode_exists(dnode, "pool-name"))
		vty_out(vty, " nat-pool %s", yang_dnode_get_string(dnode, "pool-name"));

	vty_out(vty, "\n");
}

DEFPY_YANG (
	vtysh_mgt_persistence,
	vtysh_mgt_persistence_cmd,
	"vlb persistence-group NAME mode source-ip prefix-length (0-32) [timeout (30-65535)]",
	VLB_STR
	"Persistence-group configuration\n"
	"The name of the persistence group\n"
	"Mode-specific parameters of the presistence-group\n"
	"Source IP mode\n"
	"The length of the subnet prefix\n"
	"Prefix length\n"
	"Persistence timeout configuration\n"
	"Persistence timeout interval in seconds (default 30)\n")
{
	nb_cli_enqueue_change(vty, ".", NB_OP_MODIFY, NULL);
	nb_cli_enqueue_change(vty, "./source-ip", NB_OP_MODIFY, NULL);
	nb_cli_enqueue_change(vty, "./source-ip/prefix-length", NB_OP_MODIFY, prefix_length_str);
	if (timeout_str) {
		nb_cli_enqueue_change(vty, "./timeout", NB_OP_MODIFY, timeout_str);
	} else {
		nb_cli_enqueue_change(vty, "./timeout", NB_OP_DESTROY, NULL);
	}
	return nb_cli_apply_changes(vty, "/nfware-adc:adc/persistence-groups/persistence-group[name='%s']", persistence_group);
}

DEFPY_YANG (
	vtysh_mgt_no_persistence,
	vtysh_mgt_no_persistence_cmd,
	"no vlb persistence-group NAME [mode source-ip prefix-length (0-32)]",
	NO_STR
	VLB_STR
	"Persistence-group configuration\n"
	"The name of the persistence group\n"
	"Mode-specific parameters of the presistence-group\n"
	"Source IP mode\n"
	"The length of the subnet prefix\n"
	"Prefix length\n")
{
	nb_cli_enqueue_change(vty, ".", NB_OP_DESTROY, NULL);
	return nb_cli_apply_changes(vty, "/nfware-adc:adc/persistence-groups/persistence-group[name='%s']", persistence_group);
}

static int
vlb_persistence_group_cli_cmp(const struct lyd_node *dnode1, const struct lyd_node *dnode2)
{
	const char *name1 = yang_dnode_get_string(dnode1, "name");
	const char *name2 = yang_dnode_get_string(dnode2, "name");

	return strcmp(name1, name2);
}

static void
vlb_persistence_group_cli_show(struct vty *vty, const struct lyd_node *dnode, bool show_defaults)
{
	const char *name = yang_dnode_get_string(dnode, "name");

	vty_out(vty, "vlb persistence-group %s", name);

	if (yang_dnode_exists(dnode, "source-ip")) {
		vty_out(vty, " mode source-ip prefix-length %s", yang_dnode_get_string(dnode, "source-ip/prefix-length"));
	}

	if (show_defaults || !yang_dnode_is_default(dnode, "timeout"))
		vty_out(vty, " timeout %s", yang_dnode_get_string(dnode, "timeout"));		

	vty_out(vty, "\n");
}

DEFPY_YANG (
	vtysh_mgt_service,
	vtysh_mgt_service_cmd,
	"vlb service NAME ip A.B.C.D port (0-65535) type <tcp|udp|tcp-proxy>$type target-group NAME [persistence-group NAME]",
	VLB_STR
	"Service configuration\n"
	"The name of the service\n"
	"IPv4 address of the service\n"
	"IPv4 address of the service\n"
	"L4 port of the service\n"
	"L4 port of the service\n"
	"Application-specific parameters for the service\n"
	"TCP service\n"
	"UDP service\n"
	"TCP-Proxy service\n"
	"Traffic will be balanced between workers of the default target-group\n"
	"Target-group name\n"
	"Persistence-group for the application\n"
	"Persistence-group name\n")
{
	nb_cli_enqueue_change(vty, ".", NB_OP_MODIFY, NULL);
	nb_cli_enqueue_change(vty, "./ip", NB_OP_MODIFY, ip_str);
	nb_cli_enqueue_change(vty, "./port", NB_OP_MODIFY, port_str);
	if (strmatch(type, "tcp")) {
		nb_cli_enqueue_change(vty, "./tcp", NB_OP_MODIFY, NULL);
		nb_cli_enqueue_change(vty, "./tcp/default-target-group", NB_OP_MODIFY, target_group);
		if (persistence_group) {
			nb_cli_enqueue_change(vty, "./tcp/persistence-group", NB_OP_MODIFY, persistence_group);
		} else {
			nb_cli_enqueue_change(vty, "./tcp/persistence-group", NB_OP_DESTROY, NULL);
		}
	} else if (strmatch(type, "udp")) {
		nb_cli_enqueue_change(vty, "./udp", NB_OP_MODIFY, NULL);
		nb_cli_enqueue_change(vty, "./udp/default-target-group", NB_OP_MODIFY, target_group);
		if (persistence_group) {
			nb_cli_enqueue_change(vty, "./udp/persistence-group", NB_OP_MODIFY, persistence_group);
		} else {
			nb_cli_enqueue_change(vty, "./udp/persistence-group", NB_OP_DESTROY, NULL);
		}
	} else {
		nb_cli_enqueue_change(vty, "./tcp-proxy", NB_OP_MODIFY, NULL);
		nb_cli_enqueue_change(vty, "./tcp-proxy/default-target-group", NB_OP_MODIFY, target_group);
		if (persistence_group) {
			nb_cli_enqueue_change(vty, "./tcp-proxy/persistence-group", NB_OP_MODIFY, persistence_group);
		} else {
			nb_cli_enqueue_change(vty, "./tcp-proxy/persistence-group", NB_OP_DESTROY, NULL);
		}
	}
	return nb_cli_apply_changes(vty, "/nfware-adc:adc/services/service[name='%s']", service);
}

DEFPY_YANG (
	vtysh_mgt_no_service,
	vtysh_mgt_no_service_cmd,
	"no vlb service NAME [ip A.B.C.D port (0-65535) type <tcp|udp|tcp-proxy>$type target-group NAME [persistence-group NAME]]",
	NO_STR
	VLB_STR
	"Service configuration\n"
	"The name of the service\n"
	"IPv4 address of the service\n"
	"IPv4 address of the service\n"
	"L4 port of the service\n"
	"L4 port of the service\n"
	"Application-specific parameters for the service\n"
	"TCP service\n"
	"UDP service\n"
	"TCP-Proxy service\n"
	"Traffic will be balanced between workers of the default target-group\n"
	"Target-group name\n"
	"Persistence-group for the application\n"
	"Persistence-group name\n")
{
	nb_cli_enqueue_change(vty, ".", NB_OP_DESTROY, NULL);
	return nb_cli_apply_changes(vty, "/nfware-adc:adc/services/service[name='%s']", service);
}

static int
vlb_service_cli_cmp(const struct lyd_node *dnode1, const struct lyd_node *dnode2)
{
	const char *name1 = yang_dnode_get_string(dnode1, "name");
	const char *name2 = yang_dnode_get_string(dnode2, "name");

	return strcmp(name1, name2);
}

static void
vlb_service_cli_show(struct vty *vty, const struct lyd_node *dnode, bool show_defaults)
{
	const char *name = yang_dnode_get_string(dnode, "name");
	const char *ip = yang_dnode_get_string(dnode, "ip");
	const char *port = yang_dnode_get_string(dnode, "port");

	vty_out(vty, "vlb service %s ip %s port %s", name, ip, port);

	if (yang_dnode_exists(dnode, "tcp")) {
		vty_out(vty, " type tcp target-group %s", yang_dnode_get_string(dnode, "tcp/default-target-group"));
		if (yang_dnode_exists(dnode, "tcp/persistence-group"))
			vty_out(vty, " persistence-group %s", yang_dnode_get_string(dnode, "tcp/persistence-group"));
	} else if (yang_dnode_exists(dnode, "udp")) {
		vty_out(vty, " type udp target-group %s", yang_dnode_get_string(dnode, "udp/default-target-group"));
		if (yang_dnode_exists(dnode, "udp/persistence-group"))
			vty_out(vty, " persistence-group %s", yang_dnode_get_string(dnode, "udp/persistence-group"));
	} else if (yang_dnode_exists(dnode, "tcp-proxy")) {
		vty_out(vty, " type tcp-proxy target-group %s", yang_dnode_get_string(dnode, "tcp-proxy/default-target-group"));
		if (yang_dnode_exists(dnode, "tcp-proxy/persistence-group"))
			vty_out(vty, " persistence-group %s", yang_dnode_get_string(dnode, "tcp-proxy/persistence-group"));
	}

	vty_out(vty, "\n");
}

DEFPY_YANG (
	vtysh_mgt_track_target_group,
	vtysh_mgt_track_target_group_cmd,
	"track (0-65535) target-group NAME workers-active <equal|greater-equal|less-equal>$condition (0-65535)$threshold",
	"Tracking system\n"
	"Track ID\n"
	"Track vLB target-group\n"
	"The name of the target group\n"
	"Track number of active target-group workers\n"
	"Number equal to threshold\n"
	"Number greater or equal to threshold\n"
	"Number less or equal to threshold\n"
	"Threshold\n")
{
	nb_cli_enqueue_change(vty, ".", NB_OP_MODIFY, NULL);
	nb_cli_enqueue_change(vty, "./track-target-group/target-group", NB_OP_MODIFY, target_group);
	nb_cli_enqueue_change(vty, "./track-target-group/condition", NB_OP_MODIFY, condition);
	nb_cli_enqueue_change(vty, "./track-target-group/threshold", NB_OP_MODIFY, threshold_str);
	return nb_cli_apply_changes(vty, "/nfware-adc:adc/tracks/track[id='%s']", track_str);
}

DEFPY_YANG (
	vtysh_mgt_no_track_target_group,
	vtysh_mgt_no_track_target_group_cmd,
	"no track (0-65535) [target-group NAME workers-active <equal|greater-equal|less-equal> (0-65535)]",
	NO_STR
	"Tracking system\n"
	"Track ID\n"
	"Track vLB target-group\n"
	"The name of the target group\n"
	"Track number of active target-group workers\n"
	"Number equal to threshold\n"
	"Number greater or equal to threshold\n"
	"Number less or equal to threshold\n"
	"Threshold\n")
{
	nb_cli_enqueue_change(vty, ".", NB_OP_DESTROY, NULL);
	return nb_cli_apply_changes(vty, "/nfware-adc:adc/tracks/track[id='%s']", track_str);
}

static int
vlb_track_cli_cmp(const struct lyd_node *dnode1, const struct lyd_node *dnode2)
{
	uint16_t id1 = yang_dnode_get_uint16(dnode1, "id");
	uint16_t id2 = yang_dnode_get_uint16(dnode2, "id");

	return (int)id1 - (int)id2;
}

static void
vlb_track_cli_show(struct vty *vty, const struct lyd_node *dnode, bool show_defaults)
{
	const char *id = yang_dnode_get_string(dnode, "id");

	vty_out(vty, "track %s", id);

	if (yang_dnode_exists(dnode, "track-target-group")) {
		vty_out(vty, " target-group %s workers-active %s %s",
			yang_dnode_get_string(dnode, "track-target-group/target-group"),
			yang_dnode_get_string(dnode, "track-target-group/condition"),
			yang_dnode_get_string(dnode, "track-target-group/threshold"));
	}

	vty_out(vty, "\n");
}

struct frr_yang_module_info frr_dataplane_info = {
	.name = "nfware-adc",
	.ignore_cbs = true,
	.nodes = {
		{
			.xpath = "/nfware-adc:adc/target-groups/target-group",
			.cbs = {
				.cli_cmp = vlb_target_group_cli_cmp,
				.cli_show = vlb_target_group_cli_show,
			}
		},
		{
			.xpath = "/nfware-adc:adc/target-groups/target-group/workers/worker",
			.cbs = {
				.cli_cmp = vlb_target_group_worker_cli_cmp,
				.cli_show = vlb_target_group_worker_cli_show,
			}
		},
		{
			.xpath = "/nfware-adc:adc/persistence-groups/persistence-group",
			.cbs = {
				.cli_cmp = vlb_persistence_group_cli_cmp,
				.cli_show = vlb_persistence_group_cli_show,
			}
		},
		{
			.xpath = "/nfware-adc:adc/services/service",
			.cbs = {
				.cli_cmp = vlb_service_cli_cmp,
				.cli_show = vlb_service_cli_show,
			}
		},
		{
			.xpath = "/nfware-adc:adc/tracks/track",
			.cbs = {
				.cli_cmp = vlb_track_cli_cmp,
				.cli_show = vlb_track_cli_show,
			}
		},
		{
			.xpath = NULL,
		}
	}
};

void dataplane_vty_init(void);

void
dataplane_vty_init(void)
{
	install_element(CONFIG_NODE, &vtysh_mgt_target_group_cmd);
	install_element(CONFIG_NODE, &vtysh_mgt_no_target_group_cmd);
	install_element(CONFIG_NODE, &vtysh_mgt_target_group_worker_cmd);
	install_element(CONFIG_NODE, &vtysh_mgt_no_target_group_worker_cmd);
	install_element(CONFIG_NODE, &vtysh_mgt_persistence_cmd);
	install_element(CONFIG_NODE, &vtysh_mgt_no_persistence_cmd);
	install_element(CONFIG_NODE, &vtysh_mgt_service_cmd);
	install_element(CONFIG_NODE, &vtysh_mgt_no_service_cmd);
	install_element(CONFIG_NODE, &vtysh_mgt_track_target_group_cmd);
	install_element(CONFIG_NODE, &vtysh_mgt_no_track_target_group_cmd);
}
