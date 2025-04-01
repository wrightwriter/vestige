import os
import subprocess
import shutil
import sys

arguments_string = ' '.join(sys.argv[1:])
arguments_string = ' --no-renaming '
arguments_string += ' --no-inlining '

shader_dir = r'src\shaders'

inl_dir = os.path.join(shader_dir, 'inl_files')

if os.path.exists(inl_dir):
    shutil.rmtree(inl_dir)  # Remove the directory and all its contents
os.makedirs(inl_dir)  # Recreate the directory

output_inl_file = os.path.join(inl_dir, 'shader_includes.h')

shader_files = []

for file_name in os.listdir(shader_dir):
    if file_name.endswith('.frag') or file_name.endswith('.comp'):
        shader_files.append(f'"{os.path.join(shader_dir, file_name)}"')
#USAGE: Shader Minifier [--help] [-o <string>] [-v] [--hlsl]
#                       [--format <text|indented|c-variables|c-array|js|nasm|rust>]
#                       [--field-names <rgba|xyzw|stpq>] [--preserve-externals]
#                       [--preserve-all-globals] [--no-inlining]
#                       [--aggressive-inlining] [--no-renaming]
#                       [--no-renaming-list <string>] [--no-sequence]
#                       [--no-remove-unused]
#                       [--move-declarations] [<filename>...]

full_command = f'shader_minifier.exe -v --preserve-externals  {arguments_string}  -o "{output_inl_file}" {" ".join(shader_files)}'
#full_command = f'shader_minifier.exe -v {arguments_string}  -o "{output_inl_file}" {" ".join(shader_files)}'
print(full_command)

subprocess.run(full_command, shell=True, check=True)

with open(output_inl_file, 'r') as file:
    file_data = file.read()

file_data = file_data.replace('const char *', 'static const char* ')

with open(output_inl_file, 'w') as file:
    file.write(file_data)

output_header = os.path.join(shader_dir, 'all_shaders.h')

with open(output_header, 'w') as header_file:
    header_file.write('// Automatically generated shader includes\n\n')
    header_file.write(f'#include "inl_files/shader_includes.h"\n')

print(f'Header file {output_header} has been generated successfully.')


def ensure_newlines(file_path):
    with open(file_path, "r") as file:
        lines = file.readlines()

    modified_lines = []

    for line in lines:
        stripped_line = line.rstrip()
        if stripped_line.endswith('"') and not stripped_line.endswith('\\n"'):
            modified_lines.append(stripped_line[:-1] + '\\n"\n')  # Add '\n' before closing quote
        else:
            modified_lines.append(line)

    with open(file_path, "w") as file:
        file.writelines(modified_lines)

    print(f"Processed file: {file_path}")


#ensure_newlines(output_inl_file)
