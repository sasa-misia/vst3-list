# vst3_list
A script to list the vst3 (and vst) plugins inside the main folder. Just to keep everything organized.

The only things that you have to do are: giving the main folder path, where all the plugins are stored; giving the path where you want to save output .csv files.
The script will list all the sub-folders of the companies and it will create a .csv file containing all your VSTs, grouped depending on the sub-folders.
If some plugins are listed as _Unknown, it means that these plugins are not inside a sub-folder, and you should create it (or move the plugin inside one of the existings) to keep everything organized.
If some plugins are listed as _Unrecognized, then these can be extra files or just trash.
_Unknown and _Unrecognized groups are listed in VST3_2Check.csv, all the other plugins are listed inside VST3_List.csv
