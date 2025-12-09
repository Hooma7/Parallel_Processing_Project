import numpy as np

eager_messages = []
for i in range(11):
    eager_messages.append([1, 2**i * 8])

rend_messages = []
for i in range(5):
    rend_messages.append([1, 2**(i+11) * 8])

intra_eager_timings = [
    0.000000362,
    0.000000346,
    0.000000445,
    0.000000368,
    0.000001060,
    0.000000844,
    0.000001111,
    0.000001386,
    0.000001783,
    0.000002528,
    0.000002869
]
intra_rend_timings = [
    0.000004423,
    0.000007206,
    0.000012167,
    0.000021183,
    0.000009738
]

inter_eager_timings = [
    0.000001389,
    0.000001365,
    0.000001416,
    0.000001610,
    0.000001677,
    0.000002492,
    0.000001922,
    0.000002104,
    0.000002772,
    0.000003336,
    0.000004232
]
inter_rend_timings = [
    0.000004341,
    0.000005338,
    0.000007187,
    0.000010535,
    0.000016322
]


eager_messages = np.array(eager_messages)
inter_eager_timings = np.array(inter_eager_timings)
intra_eager_timings = np.array(intra_eager_timings)

rend_messages = np.array(rend_messages)
inter_rend_timings = np.array(inter_rend_timings)
intra_rend_timings = np.array(intra_rend_timings)

vec, _, _, _ = np.linalg.lstsq(eager_messages, intra_eager_timings, rcond=None)
eager_alpha, eager_beta = vec
eager_R_p = 1.0 / eager_beta

vec, _, _, _ = np.linalg.lstsq(rend_messages, intra_rend_timings, rcond=None)
rend_alpha, rend_beta = vec
rend_R_p = 1.0 / rend_beta


vec, _, _, _ = np.linalg.lstsq(eager_messages, inter_eager_timings, rcond=None)
_, eager_beta = vec
eager_R_N = 1.0 / eager_beta

vec, _, _, _ = np.linalg.lstsq(rend_messages, inter_rend_timings, rcond=None)
_, rend_beta = vec
rend_R_N = 1.0 / rend_beta

print(f"Eager Alpha: {eager_alpha}\t\t\tEager R_p: {eager_R_p}\t\t\tEager R_N: {eager_R_N}")
print(f"Rendezvous Alpha: {rend_alpha}\t\t\tRendezvous R_p: {rend_R_p}\t\tRendezvous R_N: {rend_R_N}")
 