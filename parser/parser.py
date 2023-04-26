import argparse
import re
import numpy as np

parser = argparse.ArgumentParser(description='Usage')
parser.add_argument('--job', required=True, help='The result number to parse')
args = parser.parse_args()

f = open(args.job, 'r')

sp = args.job.split('results/')
origin = sp[1]

h_utils = []
q_utils = []
ovr_utils = []

while True:
    line = f.readline()
    if not line: break
    if line.find('H.Util') != -1:
        h_util = re.search('H.Util:(.*?)\n', line)
        if h_util is not None:
            h_utils.append(str(h_util.group(1)))
    if line.find('Q.Util') != -1:
        q_util = re.search('Q.Util:(.*?)\n', line)
        if q_util is not None:
            q_utils.append(str(q_util.group(1)))
    if line.find('Ovr.Util') != -1:
        ovr_util = re.search('Ovr.Util:(.*?)\n', line)
        if ovr_util is not None:
            ovr_utils.append(str(ovr_util.group(1)))
f.close()

fn_h_util = './parsed_results/h_util/h_util-' + origin
with open(fn_h_util, 'w') as f2:
    f2.write('\n'.join(h_utils))
fn_q_util = './parsed_results/q_util/q_util-' + origin
with open(fn_q_util, 'w') as f3:
    f3.write('\n'.join(q_utils))
fn_ovr_util = './parsed_results/ovr_util/ovr_util-' + origin
with open(fn_ovr_util, 'w') as f4:
    f4.write('\n'.join(ovr_utils))
