import os
import subprocess

source_directory = "src/shaders/"
output_directory = "data/shaders/"

if not os.path.exists(output_directory):
    os.makedirs(output_directory)

for source_file in os.listdir(source_directory):
    source_file_path = source_directory + source_file
    output_file_path = output_directory + source_file + '.spv'
    if subprocess.call("glslangvalidator -V {} -o {}".format(source_file_path, output_file_path), shell=True) != 0:
        break
