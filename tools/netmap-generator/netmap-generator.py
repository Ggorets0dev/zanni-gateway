#!/usr/bin/env python3
import argparse
import yaml
import sys
import os

def main():
    parser = argparse.ArgumentParser(description='Generate netmap header from YAML.')
    parser.add_argument('-i', '--input', required=True, help='Input YAML file')
    parser.add_argument('-o', '--output', required=True, help='Output header file (.h)')

    args = parser.parse_args()
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    template_path = os.path.join(script_dir, 'netmap.h.in')

    if not os.path.exists(args.input):
        print(f"Error: Input file '{args.input}' not found.")
        sys.exit(1)
        
    if not os.path.exists(template_path):
        print(f"Error: Template file '{template_path}' not found.")
        sys.exit(1)

    try:
        with open(args.input, 'r') as f:
            data = yaml.safe_load(f)
    except Exception as e:
        print(f"Error reading input file: {e}")
        sys.exit(1)
        
    try:
        with open(template_path, 'r') as f:
            template = f.read()
    except Exception as e:
        print(f"Error reading template file: {e}")
        sys.exit(1)

    nodes = data.get('nodes', [])
    virt_devs = data.get('virtual_devices', [])

    nodes_count = len(nodes)
    devs_count = len(virt_devs)

    # Format nodes
    nodes_str = []
    for node in nodes:
        nodes_str.append(f'{{ "{node["ip"]}", {node["port"]}, {node["id"]} }}')
    nodes_out = ",\n    ".join(nodes_str)
            
    # Format devices
    devs_str = []
    for dev in virt_devs:
        dev_type = dev["type"].upper()
        devs_str.append(f'{{ "{dev["tag"]}", {dev["node_id"]}, ZANNI_VIRT_DEV_{dev_type} }}')
    devs_out = ",\n    ".join(devs_str)

    # Replace placeholders
    output = template
    output = output.replace('@NODES_COUNT', str(nodes_count))
    output = output.replace('@DEVS_COUNT', str(devs_count))
    output = output.replace('@NODES', nodes_out)
    output = output.replace('@DEVS', devs_out)

    try:
        # Create output directory if it doesn't exist
        os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
        with open(args.output, 'w') as f:
            f.write(output)
        print(f"Successfully generated {args.output}")
    except Exception as e:
        print(f"Error writing output file: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
