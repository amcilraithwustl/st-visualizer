import json
import math
import os
import sys

import matplotlib.patches as mpatches
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import scanpy as sc
import squidpy as sq

paste1_path = "/Users/yin/Code/python/paste1/src"
paste2_path = "/Users/yin/Code/python/paste2/src"

sys.path.append(paste1_path)
sys.path.append(paste2_path)
from paste.PASTE import pairwise_align
from paste.visualization import plot_slice, stack_slices_pairwise
from paste2.PASTE2 import partial_pairwise_align, partial_pairwise_align_histology

# Set parameters
slices = [
    "/Users/yin/Code/masters-project/mouse-kidney/Spaceranger/20201201-NMK-F/20201201-NMK-F-Fc1U1Z1B1/outs",
    "/Users/yin/Code/masters-project/mouse-kidney/Spaceranger/20201201-NMK-F/20201201-NMK-F-Fc1U2Z1B1/outs",
    "/Users/yin/Code/masters-project/mouse-kidney/Spaceranger/20201201-NMK-F/20201201-NMK-F-Fc1U3Z1B1/outs",
    "/Users/yin/Code/masters-project/mouse-kidney/Spaceranger/20201201-NMK-F/20201201-NMK-F-Fc1U4Z1B1/outs"
]
slice_names = [
    "20201201-NMK-F-Fc1U1Z1B1",
    "20201201-NMK-F-Fc1U2Z1B1",
    "20201201-NMK-F-Fc1U3Z1B1",
    "20201201-NMK-F-Fc1U4Z1B1"
]
num_slices = len(slices)
slide1_path = "/Users/yin/Code/masters-project/mouse-kidney/Spaceranger/20201201-NMK-F/20201201-NMK-F-Fc1U1Z1B1/outs"
slide2_path = "/Users/yin/Code/masters-project/mouse-kidney/Spaceranger/20201201-NMK-F/20201201-NMK-F-Fc1U2Z1B1/outs"
slide1_name = "20201201-NMK-F-Fc1U1Z1B1"
slide2_name = "20201201-NMK-F-Fc1U2Z1B1"
out_root = "/Users/yin/Code/tmp/paste_test"
run_part = "1"
max_iter = 1000
args_paste = {
    "alpha": 0.1,
    "cost": "kl",
}
spot_size = 70
mode = "r"


def angle_from_sin_cos(sin_val, cos_val):
    angle_rad = math.atan2(sin_val, cos_val)
    angle_deg = math.degrees(angle_rad)
    # angle_deg = (angle_deg + 360) % 360
    return angle_deg


def transformation_matrix_process(transformation):
    sin = transformation[1, 0]
    cos = transformation[0, 0]
    angle = angle_from_sin_cos(sin, cos)
    sx = transformation[0, 2]
    sy = transformation[1, 2]
    px = ((sx + sy) * (sin + cos) + (sx - sy) * (cos - sin)) / 2
    py = ((sx + sy) * (cos - sin) - (sx - sy) * (cos + sin)) / 2
    return angle, px, py


# Detect mirroring/flip
# Seems unnecessary
# TODO: assumes general position
def mirror_check(before, after):
    det_mult = np.linalg.det(before) * np.linalg.det(after)
    return det_mult < 0


def compute_transformation(df_coord):
    random_3_rows = df_coord.sample(n=3)
    ones = np.asmatrix(np.ones(3))
    before = np.concatenate((random_3_rows[["pxl_row_in_fullres", "pxl_col_in_fullres"]].to_numpy().T, ones),
                            axis=0)
    after = np.concatenate((random_3_rows[["pxl_row_in_fullres_new", "pxl_col_in_fullres_new"]].to_numpy().T, ones),
                           axis=0)
    transformation = np.matmul(after, np.linalg.inv(before))
    transformation[2, 0] = 0.0
    return transformation


# Load and Preprocess slices
def load_and_preprocess(sample_path: str):
    visium_sample = sq.read.visium(sample_path)
    sc.pp.filter_genes(visium_sample, min_counts=15)
    sc.pp.filter_cells(visium_sample, min_counts=100)
    # make variable name unique
    visium_sample.var_names_make_unique()
    return visium_sample


def preprocess(path1, path2, name1, name2):
    # create sample name
    sample_run_name = f"{name1}_{name2}"
    print(f"Running PASTE for {sample_run_name}")

    # create out_path
    out_path = f"{out_root}/{sample_run_name}"
    if not os.path.exists(out_path):
        os.makedirs(out_path)

    # Load visium data
    slice1 = load_and_preprocess(slide1_path)
    slice2 = load_and_preprocess(slide2_path)

    return slice1, slice2, out_path


##############################################
# PASTE1
##############################################
# Pairwise alignment
# Pairwise align the slices
def run_paste_1(slice1, slice2, name1, name2, mode, out_path):
    # Compute and save pairwise alignment
    if mode.find("w") != -1:
        print("Running pairwise alignment from PASTE1")
        pi12_paste1 = pairwise_align(
            slice1,
            slice2,
            alpha=args_paste["alpha"],
            dissimilarity=args_paste["cost"],
            numItermax=max_iter,
        )
        np.save(f"{out_path}/1_PASTE1_pairwise_numpy.npy", pi12_paste1)
    # Load alignment results from npy
    elif mode.find("r") != -1:
        pi12_paste1 = np.load(f"{out_path}/1_PASTE1_pairwise_numpy.npy")
    else:
        print("No mode specified, exiting...")
        exit()

    # Plot ------------------------------------------------
    # To visualize the alignment you can stack the slices
    # according to the alignment pi
    slices, pis = [slice1, slice2], [pi12_paste1]
    new_slices = stack_slices_pairwise(slices, pis)

    slice_colors = ["#e41a1c", "#377eb8"]
    plt.figure(figsize=(7, 7))
    for i in range(len(new_slices)):
        plot_slice(new_slices[i], slice_colors[i], s=spot_size)  # s is spot size

    plt.legend(
        handles=[
            mpatches.Patch(color=slice_colors[0], label=name1),
            mpatches.Patch(color=slice_colors[1], label=name2),
        ]
    )
    plt.gca().invert_yaxis()
    plt.axis("off")
    # plt.show()
    plt.savefig(f"{out_path}/1_PASTE1_pairwise.pdf", format="pdf", bbox_inches="tight")

    n_slices = 2
    name_list = [name1, name2]
    transformations = []
    coords = []
    for i in range(n_slices):
        df_tissue = slices[i].obs[
            ["in_tissue", "array_row", "array_col"]
        ]
        df_coord_orig = pd.DataFrame(
            slices[i].obsm["spatial"],
            index=df_tissue.index,
            columns=["pxl_row_in_fullres", "pxl_col_in_fullres"],
        )
        df_coord_new = pd.DataFrame(
            new_slices[i].obsm["spatial"],
            index=df_tissue.index,
            columns=["pxl_row_in_fullres_new", "pxl_col_in_fullres_new"],
        )
        df_coord = (
            pd.concat([df_tissue, df_coord_orig, df_coord_new], axis=1)
            .reset_index()
            .rename(columns={"index": "barcode"})
        )
        df_coord.to_csv(
            f"{out_path}/1_PASTE1_pairwise_"
            + name_list[i]
            + "_new_tissue_positions.csv",
            index=False,
        )
        transformation = compute_transformation(df_coord)
        transformations.append(transformation)
        coords.append(df_coord)

    step1 = np.linalg.inv(transformations[0])
    step2 = transformations[1]
    # Might need to switch order of the two steps
    final_transformation = np.matmul(step1, step2)
    final_transformation[2, 0] = 0.0
    theta, px, py = transformation_matrix_process(final_transformation)

    json_ouput = json.dumps({
        "theta": theta,
        "px": px,
        "py": py
    })
    with open(f"{out_path}/1_PASTE1_pairwise_transformation_{name1}_{name2}.json", "w") as outfile:
        outfile.write(json_ouput)


##############################################
# Use PASTE2
##############################################
# preprocess
# first add rgb to slices


"""
    Optimal partial alignment of two slices using both gene expression and histological image information.

    sliceA, sliceB must be AnnData objects that contain .obsm['rgb'], which stores the RGB value of each spot in the histology image.
"""


def run_paste_2_partial_pairwise(slice1, slice2, name1, name2, mode, out_path):
    slice1.obsm["rgb"] = slice1.obsm["spatial"]
    slice2.obsm["rgb"] = slice2.obsm["spatial"]
    # Check if output file exists already
    output_part2_path = f"{out_path}/2_PASTE2_partial_pairwise_numpy.npy"
    if mode.find("w") != -1:
        print("Running pairwise alignment from PASTE2")
        pi12_paste2_pairwise = partial_pairwise_align(
            slice1, slice2, s=args_paste["alpha"], dissimilarity=args_paste["cost"]
        )
        np.save(output_part2_path, pi12_paste2_pairwise)
    elif mode.find("r") != -1:
        pi12_paste2_pairwise = np.load(output_part2_path)
    else:
        print("No mode specified, exiting...")
        exit()

    # PLOT ------------------------------------------------
    # To visualize the alignment you can stack the slices
    # according to the alignment pi
    slices, pis = [slice1, slice2], [pi12_paste2_pairwise]
    new_slices = stack_slices_pairwise(slices, pis)

    slice_colors = ["#e41a1c", "#377eb8"]
    plt.figure(figsize=(7, 7))
    for i in range(len(new_slices)):
        plot_slice(new_slices[i], slice_colors[i], s=spot_size)

    plt.legend(
        handles=[
            mpatches.Patch(color=slice_colors[0], label=name1),
            mpatches.Patch(color=slice_colors[1], label=name2),
        ]
    )
    plt.gca().invert_yaxis()
    plt.axis("off")
    # plt.show()
    plt.savefig(
        f"{out_path}/2_PASTE2_partial_pairwise.pdf", format="pdf", bbox_inches="tight"
    )

    n_slices = 2
    name_list = [name1, name2]
    transformations = []
    coords = []
    for i in range(n_slices):
        df_tissue = slices[i].obs[
            ["in_tissue", "array_row", "array_col"]
        ]
        df_coord_orig = pd.DataFrame(
            slices[i].obsm["spatial"],
            index=df_tissue.index,
            columns=["pxl_row_in_fullres", "pxl_col_in_fullres"],
        )
        df_coord_new = pd.DataFrame(
            new_slices[i].obsm["spatial"],
            index=df_tissue.index,
            columns=["pxl_row_in_fullres_new", "pxl_col_in_fullres_new"],
        )
        df_coord = (
            pd.concat([df_tissue, df_coord_orig, df_coord_new], axis=1)
            .reset_index()
            .rename(columns={"index": "barcode"})
        )
        df_coord.to_csv(
            f"{out_path}/2_PASTE2_partial_pairwise_"
            + name_list[i]
            + "_new_tissue_positions.csv",
            index=False,
        )
        transformation = compute_transformation(df_coord)
        transformations.append(transformation)
        coords.append(df_coord)

    step1 = np.linalg.inv(transformations[0])
    step2 = transformations[1]
    # Might need to switch order of the two steps
    final_transformation = np.matmul(step1, step2)
    final_transformation[2, 0] = 0.0
    theta, px, py = transformation_matrix_process(final_transformation)
    json_ouput = json.dumps({
        "theta": theta,
        "px": px,
        "py": py
    })
    with open(f"{out_path}/2_PASTE2_partial_pairwise_transformation_{name1}_{name2}.json", "w") as outfile:
        outfile.write(json_ouput)


##############################################
# PASTE2 algnment using histology
##############################################
def run_paste_2_partial_pairwise_histology(slice1, slice2, name1, name2, mode, out_path):
    slice1.obsm["rgb"] = slice1.obsm["spatial"]
    slice2.obsm["rgb"] = slice2.obsm["spatial"]
    npy_out_part3_path = f"{out_path}/3_PASTE2_partial_pairwise_histology_numpy.npy"
    if mode.find("w") != -1:
        print("Running pairwise alignment from PASTE2 using histology")
        slice12_al_hist = partial_pairwise_align_histology(
            slice1, slice2, s=args_paste["alpha"], dissimilarity=args_paste["cost"]
        )
        np.save(npy_out_part3_path, slice12_al_hist)
    elif mode.find("r") != -1:
        slice12_al_hist = np.load(npy_out_part3_path, allow_pickle=True)
    else:
        print("No mode specified, exiting...")
        exit()

    # Plot ------------------------------------------------
    # To visualize the alignment you can stack the slices
    # according to the alignment pi
    slices, pis = [slice1, slice2], [slice12_al_hist]
    new_slices = stack_slices_pairwise(slices, pis)

    slice_colors = ["#e41a1c", "#377eb8"]
    plt.figure(figsize=(7, 7))
    for i in range(len(new_slices)):
        plot_slice(new_slices[i], slice_colors[i], s=spot_size)

    plt.legend(
        handles=[
            mpatches.Patch(color=slice_colors[0], label=name1),
            mpatches.Patch(color=slice_colors[1], label=name2),
        ]
    )
    plt.gca().invert_yaxis()
    plt.axis("off")
    plt.savefig(
        f"{out_path}/3_PASTE2_partial_pairwise_histology.pdf",
        format="pdf",
        bbox_inches="tight",
    )

    n_slices = 2
    name_list = [name1, name2]
    transformations = []
    coords = []
    for i in range(n_slices):
        df_tissue = slices[i].obs[
            ["in_tissue", "array_row", "array_col"]
        ]
        df_coord_orig = pd.DataFrame(
            slices[i].obsm["spatial"],
            index=df_tissue.index,
            columns=["pxl_row_in_fullres", "pxl_col_in_fullres"],
        )
        df_coord_new = pd.DataFrame(
            new_slices[i].obsm["spatial"],
            index=df_tissue.index,
            columns=["pxl_row_in_fullres_new", "pxl_col_in_fullres_new"],
        )
        df_coord = (
            pd.concat([df_tissue, df_coord_orig, df_coord_new], axis=1)
            .reset_index()
            .rename(columns={"index": "barcode"})
        )
        df_coord.to_csv(
            f"{out_path}/3_PASTE2_partial_pairwise_histology_"
            + name_list[i]
            + "_new_tissue_positions.csv",
            index=False,
        )
        transformation = compute_transformation(df_coord)
        transformations.append(transformation)
        coords.append(df_coord)

    step1 = np.linalg.inv(transformations[0])
    step2 = transformations[1]
    # Might need to switch order of the two steps
    final_transformation = np.matmul(step1, step2)
    final_transformation[2, 0] = 0.0
    theta, px, py = transformation_matrix_process(final_transformation)
    json_ouput = json.dumps({
        "theta": theta,
        "px": px,
        "py": py
    })
    with open(f"{out_path}/3_PASTE2_partial_pairwise_histology_transformation_{name1}_{name2}.json", "w") as outfile:
        outfile.write(json_ouput)


################################################

# def preprocess_all(slices, slice_names):

# TODO: Need some cleanup here
# Run PASTE1
for i in range(num_slices - 1):
    slice1, slice2, out_path = preprocess(slices[i], slices[i + 1], slice_names[i], slice_names[i + 1])
    run_paste_1(slice1, slice2, slice_names[i], slice_names[i + 1], mode, out_path)

# Run PASTE2 Partial Pairwise
for i in range(num_slices - 1):
    slice1, slice2, out_path = preprocess(slices[i], slices[i + 1], slice_names[i], slice_names[i + 1])
    run_paste_2_partial_pairwise(slice1, slice2, slice_names[i], slice_names[i + 1], mode, out_path)

# Run PASTE2 Partial Pairwise Histology
for i in range(num_slices - 1):
    slice1, slice2, out_path = preprocess(slices[i], slices[i + 1], slice_names[i], slice_names[i + 1])
    run_paste_2_partial_pairwise_histology(slice1, slice2, slice_names[i], slice_names[i + 1], mode, out_path)
