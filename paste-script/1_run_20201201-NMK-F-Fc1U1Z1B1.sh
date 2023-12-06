#! /bin/bash
# Activate conda envrionment in bash script
# eval "$(conda shell.bash hook)"
# conda activate PASTE

# --------------------------------------------------
DATA_PATH="/Users/yin/Code/masters-project/mouse-kidney/Spaceranger/20201201-NMK-F/"
# Run variables. Change these
SAMPLE_NAME="20201201-NMK-F"
# Sample list
# Sample Paths
SLIDE_NAMES=(
"20201201-NMK-F-Fc1U1Z1B1"
"20201201-NMK-F-Fc1U2Z1B1"
"20201201-NMK-F-Fc1U3Z1B1"
"20201201-NMK-F-Fc1U4Z1B1"
)
SLIDE_PATHS=(
$DATA_PATH"20201201-NMK-F-Fc1U1Z1B1/outs"
$DATA_PATH"20201201-NMK-F-Fc1U2Z1B1/outs"
$DATA_PATH"20201201-NMK-F-Fc1U3Z1B1/outs"
$DATA_PATH"20201201-NMK-F-Fc1U4Z1B1/outs"
)

# --------------------------------------------------
# Project root. Change if needed
PROJECT_ROOT="/Users/yin/Code/tmp"

# Script constants. No need to change
RUN_PASTE_SCRIPT="/Users/yin/Code/python/paste-script/1_runPASTE.py"
PASTE2_PATH="/Users/yin/Code/python/paste2/src"

# --------------------------------------------------
# Options for the 1_runPASTE.py script
#   -h, --help            show this help message and exit
#   -1 SLIDE1_PATH, --slide1_path=SLIDE1_PATH
#                         slide1 path
#   -2 SLIDE2_PATH, --slide2_path=SLIDE2_PATH
#                         slide2 path
#   -a SLIDE1_NAME, --slide1_name=SLIDE1_NAME
#                         slide1 name
#   -b SLIDE2_NAME, --slide2_name=SLIDE2_NAME
#                         slide2 name
#   -o OUT_ROOT, --out_root=OUT_ROOT
#                         output path
#   -p PASTE2_PATH, --paste2_path=PASTE2_PATH
#                         paste2 path
#   -r RUN_PART, --run_part=RUN_PART
#                         run part. 1=PASTE1, 2=PASTE2, 3=PASTE2-histology. 12=PASTE1+PASTE2. 13=PASTE1+PASTE2-histology. 23=PASTE2+PASTE2-histology. 123=PASTE1+PASTE2+PASTE2-histology")
# --------------------------------------------------
# run script
# Loop through elements in array and run script
for (( i=0; i<${#SLIDE_NAMES[@]}-1; i++ ))
do
    python3 $RUN_PASTE_SCRIPT \
    --paste2_path $PASTE2_PATH \
    --slide1_path ${SLIDE_PATHS[$i]} \
    --slide2_path ${SLIDE_PATHS[$i+1]} \
    --slide1_name ${SLIDE_NAMES[$i]} \
    --slide2_name ${SLIDE_NAMES[$i+1]} \
    --out_root "$PROJECT_ROOT/$SAMPLE_NAME" \
    --run_part 123
done
