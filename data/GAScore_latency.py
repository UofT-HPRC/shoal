import os

# This script parses the output from Vivado to compute latency numbers for the
# GAScore.

vivado_output_file = "GAScore_latency.txt"
search_string = "STAT_"
units = 'us'
clock_period = 0.02
# clock_align = 0.01 # signal transitions from sonar occur on negedges

table_template = "| {label:38} | {init:11} | {count:6} | {extra:38} |"

def start_trans(first_beat):
    return round(first_beat - clock_period, 3)

def cycle_count(base, delta, scaling=1):
    cycles = round(round(delta - base, 3) / clock_period, 3)
    return int(cycles)/scaling

def truncate_string(text_str, length):
    trunc_length = length - 3
    return text_str[:trunc_length] + '...' if len(text_str) > length else text_str

def print_stat_abs(label, first_beat, last_beat, prev_beat=None):
    label_trunc = truncate_string(label, 38)
    if prev_beat is not None:
        init_cycles = cycle_count(prev_beat, first_beat)
    else:
        init_cycles = "N/A"
    cycles = cycle_count(first_beat, last_beat)
    print(table_template.format(label=label_trunc, init=init_cycles, count=cycles, extra='N/A'))

def print_stat_offset(label, first_beat, last_beat, target=None, prev_beat=None):
    label_trunc = truncate_string(label, 38)
    if prev_beat is not None:
        init_cycles = cycle_count(prev_beat, start_trans(first_beat))
    else:
        init_cycles = "N/A"
    cycles = cycle_count(start_trans(first_beat), last_beat)
    if target is not None:
        extra = truncate_string("Should be " + str(target) + " cycles", 38)
    else:
        extra = 'N/A'
    print(table_template.format(label=label_trunc, init=init_cycles, count=cycles, extra=extra))

def create_tuples(start_index, end_index):
    # lindex = start_index + 1 # skip the initial so only intra packet beats are measured
    lindex = start_index
    rindex = lindex + 1
    tuples_list = []
    while(rindex <= end_index):
        tuples_list.append((lindex, rindex))
        lindex = rindex
        rindex += 1
    return tuples_list

def delay(stat_list, tuples_list):
    max_delay = 0
    min_delay = -1
    for tuples in tuples_list:
        current_delay = cycle_count(stat_list[tuples[0]]['time'], stat_list[tuples[1]]['time'])
        if current_delay > max_delay:
            max_delay = current_delay
        if min_delay == -1:
            min_delay = current_delay
        elif current_delay < min_delay:
            min_delay = current_delay
    return max_delay, min_delay

def print_delay(stat_list, start_index, end_index, target_count):
    # print(start_trans(stat_list[start_index]['time']))
    # print(stat_list[end_index]['time'])
    if cycle_count(start_trans(stat_list[start_index]['time']), stat_list[end_index]['time']) != target_count:
        # print(start_index, end_index)
        # print(create_tuples(start_index, end_index))
        max_delay, min_delay = delay(stat_list, create_tuples(start_index, end_index))
        # print(max_delay)
        # print(min_delay)
        if max_delay != min_delay:
            max_string = "Max delay between beats: " + str(max_delay) + " cycles"
            label_trunc = truncate_string(max_string, 38)
            print(table_template.format(label='^', init='^', count='^', extra=label_trunc))
            min_string = "Min delay between beats: " + str(min_delay) + " cycles"
        else:
            min_string = "Delay between beats: " + str(min_delay) + " cycles"
        label_trunc = truncate_string(min_string, 38)
        print(table_template.format(label='^', init='^', count='^', extra=label_trunc))

def print_header(label, filler):
    # print(table_template.format(label="", init="", count="", extra=''))
    print("| " + label + " " + filler*(101-len(label)) + " |" )
    # print(table_template.format(label="", init="", count="", extra=''))

# get relative path of the text file (same location as script)
__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))
filename = os.path.join(__location__, vivado_output_file)

stats = []
with open(filename, 'r') as f:
    for line in f:
        if line.startswith(search_string):
            line_split = line.split(':')
            stat_id_split = line_split[0].split('_')
            stat_time_split = line_split[1].split(' ')
            stat = {}
            stat['vector'] = stat_id_split[1]
            stat['thread'] = int(stat_id_split[2])
            stat['index'] = int(stat_id_split[3])
            stat['time'] = float(stat_time_split[1])
            stats.append(stat)

print(table_template.format(label="Label", init="Init Cycles", count="Cycles", extra='Metadata'))

# parse short message A (sma)

fltr_stats_2 = list(filter(lambda x : x['vector'] == 'sma' and x['thread'] == 2, stats))
fltr_stats_3 = list(filter(lambda x : x['vector'] == 'sma' and x['thread'] == 3, stats))

print_header("Short Message A", "=")

print_header("Thread 1", "-")

# time for VIP to do a AXI-Lite write
print_stat_abs("Handler config register write (VIP)", fltr_stats_2[0]['time'], fltr_stats_2[1]['time'])

# time between first and last beats of incoming network SM
print_stat_offset("Incoming SM from network - third", fltr_stats_2[3]['time'], fltr_stats_2[4]['time'], 2.0)
print_delay(fltr_stats_2, 3, 4, 2.0)
print_stat_offset("Incoming SM from network - fourth", fltr_stats_2[5]['time'], fltr_stats_2[7]['time'], 3.0, fltr_stats_2[4]['time'])
print_delay(fltr_stats_2, 5, 7, 3.0)

# time for the interrupt signal to go high following the SM
print_stat_abs("Interrupt resolution", fltr_stats_2[7]['time'], fltr_stats_2[8]['time'])

print_header("Thread 2", "-")

# time between first and last beat of network reply
print_stat_offset("Outgoing reply to network - third", fltr_stats_3[1]['time'], fltr_stats_3[2]['time'], 2.0)
print_delay(fltr_stats_3, 0, 2, 2.0)
print_stat_offset("Outgoing reply to network - fourth", fltr_stats_3[3]['time'], fltr_stats_3[4]['time'], 2.0)
print_delay(fltr_stats_3, 2, 4, 2.0)

print_header("Inter-thread", "-")

# time between end of SM to beginning of reply
print_stat_abs("SM to reply delay - third", fltr_stats_2[4]['time'], fltr_stats_3[1]['time'])
print_stat_abs("SM to reply delay - fourth", fltr_stats_2[7]['time'], fltr_stats_3[3]['time'])

# parse short message B (smb)

fltr_stats_2 = list(filter(lambda x : x['vector'] == 'smb' and x['thread'] == 2, stats))
fltr_stats_3 = list(filter(lambda x : x['vector'] == 'smb' and x['thread'] == 3, stats))

print_header("Short Message B", "=")

print_header("Thread 1", "-")

print_stat_offset("Incoming SM from kernel - 1", fltr_stats_2[0]['time'], fltr_stats_2[2]['time'], 3.0)
print_delay(fltr_stats_2, 0, 2, 3.0)
print_stat_offset("Incoming SM from kernel - 2", fltr_stats_2[3]['time'], fltr_stats_2[5]['time'], 3.0, fltr_stats_2[2]['time'])
print_delay(fltr_stats_2, 3, 5, 3.0)

# time for VIP to do a AXI-Lite write
print_stat_abs("Handler config register write (VIP)", fltr_stats_2[5]['time'], fltr_stats_2[6]['time'])

print_stat_offset("Incoming SM from kernel - 3", fltr_stats_2[8]['time'], fltr_stats_2[9]['time'], 2.0, fltr_stats_2[7]['time'])
print_delay(fltr_stats_2, 8, 9, 2.0)
print_stat_offset("Incoming SM from kernel - 4", fltr_stats_2[10]['time'], fltr_stats_2[12]['time'], 3.0, fltr_stats_2[9]['time'])
print_delay(fltr_stats_2, 10, 12, 3.0)
print_stat_offset("Incoming SM from kernel for network - 5", fltr_stats_2[13]['time'], fltr_stats_2[14]['time'], 2.0, fltr_stats_2[12]['time'])
print_delay(fltr_stats_2, 13, 14, 2.0)
print_stat_offset("Incoming SM from kernel for network - 6", fltr_stats_2[15]['time'], fltr_stats_2[16]['time'], 2.0, fltr_stats_2[14]['time'])
print_delay(fltr_stats_2, 15, 16, 2.0)

print_stat_offset("Incoming SM reply from network", fltr_stats_2[20]['time'], fltr_stats_2[21]['time'], 2.0)
print_delay(fltr_stats_2, 20, 21, 2.0)

print_header("Thread 2", "-")

print_stat_offset("Outgoing to network - 7", fltr_stats_3[5]['time'], fltr_stats_3[6]['time'], 2.0)
print_delay(fltr_stats_3, 5, 6, 2.0)
print_stat_offset("Outgoing to network - 8", fltr_stats_3[7]['time'], fltr_stats_3[8]['time'], 2.0)
print_delay(fltr_stats_3, 7, 8, 2.0)

print_header("Inter-thread", "-")

# time between end of SM to beginning of reply
print_stat_abs("SM to reply delay - 1", fltr_stats_2[0]['time'], fltr_stats_3[1]['time'])
print_stat_abs("SM to reply delay - 2", fltr_stats_2[3]['time'], fltr_stats_3[2]['time'])
print_stat_abs("SM to reply delay - 3", fltr_stats_2[8]['time'], fltr_stats_3[3]['time'])
print_stat_abs("SM to reply delay - 4", fltr_stats_2[10]['time'], fltr_stats_3[4]['time'])
print_stat_abs("SM to reply delay - 5", fltr_stats_2[13]['time'], fltr_stats_3[6]['time'])
print_stat_abs("SM to reply delay - 6", fltr_stats_2[15]['time'], fltr_stats_3[8]['time'])

print_stat_abs("Net reply to kernel forward delay", fltr_stats_2[20]['time'], fltr_stats_3[10]['time'])
