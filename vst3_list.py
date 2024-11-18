# -*- coding: utf-8 -*-
"""
Created on Sun Oct 22 13:19:47 2023

@author: salva
"""

####~ Modules recall ~#########################################################
import pandas as pd
import os
from os.path import isdir, join, basename

####~ Functions ~##############################################################
def filepathinfo(curr_path, base_path):
    file_basenm = basename(curr_path)
    file_extens = os.path.splitext(curr_path)[-1].lower()
    path_prefix = os.path.commonpath([curr_path, base_path])
    path_suffix = os.path.relpath(os.path.dirname(curr_path), path_prefix)
    return file_basenm, file_extens, path_prefix, path_suffix

def pthdirnav(path_start):
    vst3_extns = ['.vst3', '.dll']
    excl_extns = ['.ini'] # excluded files
    
    list_pth_el = os.listdir(path_start)
    full_paths = [join(path_start, i) for i in list_pth_el]
    list_sub_pths, list_files = list(), list()
    for curr_path in full_paths:
        _, file_ext, _, _ = filepathinfo(curr_path, path_start)
        
        if isdir(curr_path) and not(any([x == file_ext for x in vst3_extns])): # It works also with vst2
            list_sub_pths.append(curr_path)
            
        elif any([x == file_ext for x in excl_extns]):
            continue # Nothing to do, just excluded hidden files
            
        else:
            list_files.append(curr_path)
            
    return list_sub_pths, list_files

def nameofvst(curr_path, start_path):
    vst3_extns = ['.vst3', '.dll']
    filename_vst = str()
    
    flnm_crr, file_ext, _, pth_suff = filepathinfo(curr_path, start_path)
    filename_gen = join(pth_suff, flnm_crr)
    if any([x == file_ext for x in vst3_extns]): # It works also with vst2
        filename_vst = flnm_crr
    return filename_vst, filename_gen

####~ Core ~###################################################################
nt_rc_lbl = '_Not recognized'
uknwn_lbl = '_Unknown'

path_vst3 = input('VST3 folder ([C:\\Program Files\\Common Files\\VST3]): ') or 'C:\\Program Files\\Common Files\\VST3'
path_rprt = input(f'Excel folder ([{os.getcwd()}]): ') or os.getcwd()

path_sprd = join(path_rprt, 'VST3_List.csv')
path_chck = join(path_rprt, 'VST3_2Check.csv')

temp_pths, nodir_vst = pthdirnav(path_vst3)

vst_flds_raw = {uknwn_lbl: nodir_vst}
strt_fld_raw = {uknwn_lbl: path_vst3}
for curr_path in temp_pths:
    strt_fld_raw[basename(curr_path)] = curr_path
    vst_flds_raw[basename(curr_path)] = list()
    
    temp_sub_flds = [curr_path]
    while len(temp_sub_flds) >= 1:
        tmp_subs, tmp_files = pthdirnav(temp_sub_flds[0])
        vst_flds_raw[basename(curr_path)] += tmp_files
        temp_sub_flds += tmp_subs
        temp_sub_flds.pop(0)

list_vst3 = {nt_rc_lbl: list()} # Dictionary type, initializing
for x in vst_flds_raw:
    list_vst3[x] = list()
    
for curr_dev, curr_files in vst_flds_raw.items():
    for curr_pth in curr_files:
        # temp_start_pth = strt_fld_raw[curr_dev]
        curr_vst, curr_gen = nameofvst(curr_pth, path_vst3)
        if not(len(curr_vst) == 0):
            list_vst3[curr_dev] += [curr_vst]
        else:
            list_vst3[nt_rc_lbl] += [curr_gen]

vst3_df = pd.DataFrame.from_dict(list_vst3, orient='index').transpose()

vst3_df.to_csv(path_or_buf=path_sprd, columns=vst3_df.columns[2:], index=False)
vst3_df.to_csv(path_or_buf=path_chck, columns=vst3_df.columns[:2], index=False)