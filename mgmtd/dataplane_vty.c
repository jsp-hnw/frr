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
