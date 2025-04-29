# -*- coding: utf-8 -*-
"""
Created on Sun Oct 22 13:19:47 2023

@author: salva
"""

####~ Modules recall ~#########################################################
import pandas as pd
import os
from os.path import join, basename, splitext, commonpath, relpath
# from pathlib import PurePath

####~ Constants ~##############################################################
VST3_EXTENSIONS = ['.vst3', '.dll']
EXCLUDED_EXTENSIONS = ['.ini']
NOT_RECOGNIZED_LABEL = '_Not recognized'
UNKNOWN_LABEL = '_Unknown'

####~ Functions ~##############################################################
def filepathinfo(curr_path, base_path):
    """
    Extracts file information such as basename, extension, common prefix, and relative path.
    """
    file_basenm = basename(curr_path)
    file_extens = splitext(curr_path)[-1].lower()
    path_prefix = commonpath([curr_path, base_path])
    path_suffix = relpath(os.path.dirname(curr_path), path_prefix)
    return file_basenm, file_extens, path_prefix, path_suffix

def is_subpath(parent_path, child_path):
    """
    Detect if a certain path is a subpath of another.
    """
    return child_path.startswith(tuple(parent_path))
    # parent = PurePath(parent_path) # slower alternative
    # child = PurePath(child_path)
    # try:
    #     child.relative_to(parent)
    #     return True
    # except ValueError:
    #     return False

def list_files_and_dirs(path_start):
    """
    Lists all files and directories in the given path, excluding certain extensions.
    """
    list_sub_pths, list_files = [], []  # Use a set for faster lookups
    for root, dirs, files in os.walk(path_start):
        for dir in dirs:
            path_dir = join(root, dir)
            _, file_ext, _, _ = filepathinfo(path_dir, path_start)
            if file_ext in VST3_EXTENSIONS:
                list_files.append(path_dir)
            else:
                list_sub_pths.append(path_dir)
        for file in files:
            path_file = join(root, file)
            _, file_ext, _, _ = filepathinfo(path_file, path_start)
            if file_ext in EXCLUDED_EXTENSIONS:
                continue
            else:
                # Check if the file is not inside any .vst3 folder
                if not is_subpath(list_files, path_file):
                    list_files.append(path_file)
    return list_files, list_sub_pths

def nameofvst(curr_path, start_path):
    """
    Determines the name of the VST plugin and its relative path.
    """
    filename_vst, file_ext, _, pth_suff = filepathinfo(curr_path, start_path)
    if pth_suff == '.':
        manufacturer = UNKNOWN_LABEL
    else:
        manufacturer = pth_suff
    filename_gen = join(pth_suff, filename_vst)
    if file_ext in VST3_EXTENSIONS:
        return filename_vst, filename_gen, manufacturer
    return "", filename_gen, NOT_RECOGNIZED_LABEL

def process_vst3_folder(path_vst3):
    """
    Processes the VST3 folder and organizes plugins by manufacturer.
    """
    all_files, _ = list_files_and_dirs(path_vst3)

    vst_dict = {UNKNOWN_LABEL: [], NOT_RECOGNIZED_LABEL: []}
    for curr_path in all_files:
        curr_vst_file, curr_gen_file, curr_vst_dev = nameofvst(curr_path, path_vst3)
        if curr_vst_dev not in list(vst_dict.keys()):
            vst_dict[curr_vst_dev] = []
        if curr_vst_file:
            vst_dict[curr_vst_dev].append(curr_vst_file)
        else:
            vst_dict[curr_vst_dev].append(curr_gen_file)

    return vst_dict

def save_to_csv(vst3_data, path_rprt):
    """
    Saves the VST3 data to CSV files.
    """
    path_sprd = join(path_rprt, 'VST3_List.csv')
    path_chck = join(path_rprt, 'VST3_2Check.csv')

    vst3_df = pd.DataFrame.from_dict(vst3_data, orient='index').transpose()
    vst3_df.to_csv(path_or_buf=path_sprd, columns=[col for col in vst3_df.columns if col not in [UNKNOWN_LABEL, NOT_RECOGNIZED_LABEL]], index=False)
    vst3_df.to_csv(path_or_buf=path_chck, columns=[UNKNOWN_LABEL, NOT_RECOGNIZED_LABEL], index=False)

####~ Core ~###################################################################
if __name__ == "__main__":
    """
    If you run this module as a script, then the following lines will be executed
    """
    path_vst3 = input('VST3 folder ([C:\\Program Files\\Common Files\\VST3]): ') or 'C:\\Program Files\\Common Files\\VST3'
    path_rprt = input(f'Excel folder ([{os.getcwd()}]): ') or os.getcwd()

    vst3_data = process_vst3_folder(path_vst3)
    save_to_csv(vst3_data, path_rprt)