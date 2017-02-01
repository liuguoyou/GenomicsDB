#!/usr/bin/env python

#The MIT License (MIT)
#Copyright (c) 2016 Intel Corporation

#Permission is hereby granted, free of charge, to any person obtaining a copy of 
#this software and associated documentation files (the "Software"), to deal in 
#the Software without restriction, including without limitation the rights to 
#use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of 
#the Software, and to permit persons to whom the Software is furnished to do so, 
#subject to the following conditions:

#The above copyright notice and this permission notice shall be included in all 
#copies or substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
#FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
#COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
#IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
#CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import json
import tempfile
import subprocess
import hashlib
import os
import sys
import shutil

query_json_template_string="""
{   
        "workspace" : "",
        "array" : "",
        "vcf_header_filename" : ["inputs/template_vcf_header.vcf"],
        "query_column_ranges" : [ [ [0, 10000000000 ] ] ],
        "query_row_ranges" : [ [ [0, 2 ] ] ],
        "reference_genome" : "inputs/chr1_10MB.fasta.gz",
        "query_attributes" : [ "REF", "ALT", "BaseQRankSum", "MQ", "RAW_MQ", "MQ0", "ClippingRankSum", "MQRankSum", "ReadPosRankSum", "DP", "GT", "GQ", "SB", "AD", "PL", "DP_FORMAT", "MIN_DP", "PID", "PGT" ]
}"""

vcf_query_attributes_order = [ "END", "REF", "ALT", "BaseQRankSum", "ClippingRankSum", "MQRankSum", "ReadPosRankSum", "MQ", "RAW_MQ", "MQ0", "DP", "GT", "GQ", "SB", "AD", "PL", "PGT", "PID", "MIN_DP", "DP_FORMAT" ];

def create_query_json(ws_dir, test_name, query_params_dict):
    test_dict=json.loads(query_json_template_string);
    test_dict["workspace"] = ws_dir
    test_dict["array"] = test_name
    test_dict["query_column_ranges"] = [ [ query_params_dict["query_column_ranges"] ] ]
    return test_dict;


loader_json_template_string="""
{
    "row_based_partitioning" : false,
    "column_partitions" : [
        {"begin": 0, "workspace":"", "array": "" }
    ],
    "callset_mapping_file" : "",
    "vid_mapping_file" : "inputs/vid.json",
    "size_per_column_partition": 3000,
    "treat_deletions_as_intervals" : true,
    "vcf_header_filename": "inputs/template_vcf_header.vcf",
    "reference_genome" : "inputs/chr1_10MB.fasta.gz",
    "num_parallel_vcf_files" : 1,
    "do_ping_pong_buffering" : false,
    "offload_vcf_output_processing" : false,
    "discard_vcf_index": true,
    "produce_combined_vcf": true,
    "produce_tiledb_array" : true,
    "delete_and_create_tiledb_array" : true,
    "compress_tiledb_array" : false,
    "segment_size" : 1048576,
    "num_cells_per_tile" : 3
}""";

def create_loader_json(ws_dir, test_name, test_params_dict):
    test_dict=json.loads(loader_json_template_string);
    if('column_partitions' in test_params_dict):
        test_dict['column_partitions'] = test_params_dict['column_partitions'];
    test_dict["column_partitions"][0]["workspace"] = ws_dir;
    test_dict["column_partitions"][0]["array"] = test_name;
    test_dict["callset_mapping_file"] = test_params_dict['callset_mapping_file'];
    if('vid_mapping_file' in test_params_dict):
        test_dict['vid_mapping_file'] = test_params_dict['vid_mapping_file'];
    return test_dict;

def get_file_content_and_md5sum(filename):
    with open(filename, 'rb') as fptr:
        data = fptr.read();
        md5sum_hash_str = str(hashlib.md5(data).hexdigest())
        fptr.close();
        return (data, md5sum_hash_str);

def print_diff(golden_output, test_output):
    print("=======Golden output:=======");
    print(golden_output);
    print("=======Test output:=======");
    print(test_output);
    print("=======END=======");

def cleanup_and_exit(tmpdir, exit_code):
    if(exit_code == 0):
        shutil.rmtree(tmpdir, ignore_errors=True)
    sys.exit(exit_code);

def main():
    #Switch to tests directory
    parent_dir=os.path.dirname(os.path.realpath(__file__))
    os.chdir(parent_dir)
    #Zero line coverage
    subprocess.call('lcov --directory ../ --zerocounters', shell=True);
    exe_path = '../bin/'
    tmpdir = tempfile.mkdtemp()
    ws_dir=tmpdir+os.path.sep+'ws';
    #Buffer size
    segment_size = 40
    load_segment_size = 40
    loader_tests = [
            { "name" : "t0_1_2", 'golden_output' : 'golden_outputs/t0_1_2_loading',
                'callset_mapping_file': 'inputs/callsets/t0_1_2.json',
                "query_params": [
                    { "query_column_ranges" : [0, 1000000000], "golden_output": {
                        "calls"      : "golden_outputs/t0_1_2_calls_at_0",
                        "variants"   : "golden_outputs/t0_1_2_variants_at_0",
                        "vcf"        : "golden_outputs/t0_1_2_vcf_at_0",
                        "batched_vcf": "golden_outputs/t0_1_2_vcf_at_0",
                        "java_vcf"   : "golden_outputs/java_t0_1_2_vcf_at_0",
                        } },
                    { "query_column_ranges" : [12150, 1000000000], "golden_output": {
                        "calls"      : "golden_outputs/t0_1_2_calls_at_12150",
                        "variants"   : "golden_outputs/t0_1_2_variants_at_12150",
                        "vcf"        : "golden_outputs/t0_1_2_vcf_at_12150",
                        "batched_vcf": "golden_outputs/t0_1_2_vcf_at_12150",
                        "java_vcf"   : "golden_outputs/java_t0_1_2_vcf_at_12150",
                        } }
                    ]
            },
            { "name" : "t0_1_2_csv", 'golden_output' : 'golden_outputs/t0_1_2_loading',
                'callset_mapping_file': 'inputs/callsets/t0_1_2_csv.json',
                "query_params": [
                    { "query_column_ranges" : [0, 1000000000], "golden_output": {
                        "calls"      : "golden_outputs/t0_1_2_calls_at_0",
                        "variants"   : "golden_outputs/t0_1_2_variants_at_0",
                        "vcf"        : "golden_outputs/t0_1_2_vcf_at_0",
                        "batched_vcf": "golden_outputs/t0_1_2_vcf_at_0",
                        "java_vcf"   : "golden_outputs/java_t0_1_2_vcf_at_0",
                        } },
                    { "query_column_ranges" : [12150, 1000000000], "golden_output": {
                        "calls"      : "golden_outputs/t0_1_2_calls_at_12150",
                        "variants"   : "golden_outputs/t0_1_2_variants_at_12150",
                        "vcf"        : "golden_outputs/t0_1_2_vcf_at_12150",
                        "batched_vcf": "golden_outputs/t0_1_2_vcf_at_12150",
                        "java_vcf"   : "golden_outputs/java_t0_1_2_vcf_at_12150",
                        } }
                    ]
            },
            { "name" : "t0_overlapping", 'golden_output': 'golden_outputs/t0_overlapping',
                'callset_mapping_file': 'inputs/callsets/t0_overlapping.json',
                "query_params": [
                    { "query_column_ranges" : [12202, 1000000000], "golden_output": {
                        "vcf"        : "golden_outputs/t0_overlapping_at_12202",
                        }
                    }
                ]
            },
            { "name" : "t0_overlapping_at_12202", 'golden_output': 'golden_outputs/t0_overlapping_at_12202',
                'callset_mapping_file': 'inputs/callsets/t0_overlapping.json',
                'column_partitions': [ {"begin": 12202, "workspace":"", "array": "" }]
            },
            { "name" : "t6_7_8", 'golden_output' : 'golden_outputs/t6_7_8_loading',
                'callset_mapping_file': 'inputs/callsets/t6_7_8.json',
                "query_params": [
                    { "query_column_ranges" : [0, 1000000000], "golden_output": {
                        "calls"      : "golden_outputs/t6_7_8_calls_at_0",
                        "variants"   : "golden_outputs/t6_7_8_variants_at_0",
                        "vcf"        : "golden_outputs/t6_7_8_vcf_at_0",
                        "batched_vcf": "golden_outputs/t6_7_8_vcf_at_0",
                        } },
                    { "query_column_ranges" : [8029500, 1000000000], "golden_output": {
                        "calls"      : "golden_outputs/t6_7_8_calls_at_8029500",
                        "variants"   : "golden_outputs/t6_7_8_variants_at_8029500",
                        "vcf"        : "golden_outputs/t6_7_8_vcf_at_8029500",
                        "batched_vcf": "golden_outputs/t6_7_8_vcf_at_8029500",
                        } }
                    ]
            },
            { "name" : "java_t0_1_2", 'golden_output' : 'golden_outputs/t0_1_2_loading',
                'callset_mapping_file': 'inputs/callsets/t0_1_2.json',
                "query_params": [
                    { "query_column_ranges" : [0, 1000000000], "golden_output": {
                        "calls"      : "golden_outputs/t0_1_2_calls_at_0",
                        "variants"   : "golden_outputs/t0_1_2_variants_at_0",
                        "vcf"        : "golden_outputs/t0_1_2_vcf_at_0",
                        "batched_vcf": "golden_outputs/t0_1_2_vcf_at_0",
                        "java_vcf"   : "golden_outputs/java_t0_1_2_vcf_at_0",
                        } },
                    { "query_column_ranges" : [12150, 1000000000], "golden_output": {
                        "calls"      : "golden_outputs/t0_1_2_calls_at_12150",
                        "variants"   : "golden_outputs/t0_1_2_variants_at_12150",
                        "vcf"        : "golden_outputs/t0_1_2_vcf_at_12150",
                        "batched_vcf": "golden_outputs/t0_1_2_vcf_at_12150",
                        "java_vcf"   : "golden_outputs/java_t0_1_2_vcf_at_12150",
                        } }
                    ]
            },
            { "name" : "java_buffer_stream_t0_1_2", 'golden_output' : 'golden_outputs/t0_1_2_loading',
                'callset_mapping_file': 'inputs/callsets/t0_1_2_buffer.json',
                'stream_name_to_filename_mapping': 'inputs/callsets/t0_1_2_buffer_mapping.json',
                "query_params": [
                    { "query_column_ranges" : [0, 1000000000], "golden_output": {
                        "calls"      : "golden_outputs/t0_1_2_calls_at_0",
                        "variants"   : "golden_outputs/t0_1_2_variants_at_0",
                        "vcf"        : "golden_outputs/t0_1_2_vcf_at_0",
                        "batched_vcf": "golden_outputs/t0_1_2_vcf_at_0",
                        "java_vcf"   : "golden_outputs/java_t0_1_2_vcf_at_0",
                        } },
                    { "query_column_ranges" : [12150, 1000000000], "golden_output": {
                        "calls"      : "golden_outputs/t0_1_2_calls_at_12150",
                        "variants"   : "golden_outputs/t0_1_2_variants_at_12150",
                        "vcf"        : "golden_outputs/t0_1_2_vcf_at_12150",
                        "batched_vcf": "golden_outputs/t0_1_2_vcf_at_12150",
                        "java_vcf"   : "golden_outputs/java_t0_1_2_vcf_at_12150",
                        } }
                    ]
            },
            { "name" : "java_buffer_stream_multi_contig_t0_1_2", 'golden_output' : 'golden_outputs/t0_1_2_loading',
                'callset_mapping_file': 'inputs/callsets/t0_1_2_buffer.json',
                'stream_name_to_filename_mapping': 'inputs/callsets/t0_1_2_buffer_mapping.json',
                "query_params": [
                    { "query_column_ranges" : [0, 1000000000], "golden_output": {
                        "calls"      : "golden_outputs/t0_1_2_calls_at_0",
                        "variants"   : "golden_outputs/t0_1_2_variants_at_0",
                        "vcf"        : "golden_outputs/t0_1_2_vcf_at_0",
                        "batched_vcf": "golden_outputs/t0_1_2_vcf_at_0",
                        "java_vcf"   : "golden_outputs/java_t0_1_2_vcf_at_0",
                        } },
                    { "query_column_ranges" : [12150, 1000000000], "golden_output": {
                        "calls"      : "golden_outputs/t0_1_2_calls_at_12150",
                        "variants"   : "golden_outputs/t0_1_2_variants_at_12150",
                        "vcf"        : "golden_outputs/t0_1_2_vcf_at_12150",
                        "batched_vcf": "golden_outputs/t0_1_2_vcf_at_12150",
                        "java_vcf"   : "golden_outputs/java_t0_1_2_vcf_at_12150",
                        } }
                    ]
            },
            { "name" : "test_new_fields", 'golden_output' : 'golden_outputs/t6_7_8_new_field_gatk.vcf',
                'callset_mapping_file': 'inputs/callsets/t6_7_8.json',
                'vid_mapping_file': 'inputs/vid_MLEAC_MLEAF.json'
            },
            { "name" : "test_info_combine_ops0", 'golden_output' : 'golden_outputs/info_ops0.vcf',
                'callset_mapping_file': 'inputs/callsets/info_ops.json',
                'vid_mapping_file': 'inputs/vid_info_ops0.json'
            },
            { "name" : "test_info_combine_ops1", 'golden_output' : 'golden_outputs/info_ops1.vcf',
                'callset_mapping_file': 'inputs/callsets/info_ops.json',
                'vid_mapping_file': 'inputs/vid_info_ops1.json'
            },
            { "name" : "t0_1_2_coverage", 'golden_output' : 'golden_outputs/t0_1_2_coverage',
                'callset_mapping_file': 'inputs/callsets/t0_1_2_coverage.json',
                "query_params": [
                    { "query_column_ranges" : [0, 1000000000], "golden_output": {
                        "calls"      : "golden_outputs/t0_1_2_coverage_calls_at_0",
                        "variants"      : "golden_outputs/t0_1_2_coverage_variants_at_0",
                        } },
                    ]
            }
    ];
    for test_params_dict in loader_tests:
        test_name = test_params_dict['name']
        test_loader_dict = create_loader_json(ws_dir, test_name, test_params_dict);
        if(test_name == "t0_1_2"):
            test_loader_dict["compress_tiledb_array"] = True;
        loader_json_filename = tmpdir+os.path.sep+test_name+'.json'
        test_loader_dict['segment_size'] = load_segment_size;
        with open(loader_json_filename, 'wb') as fptr:
            json.dump(test_loader_dict, fptr, indent=4, separators=(',', ': '));
            fptr.close();
        if(test_name  == 'java_t0_1_2'):
            pid = subprocess.Popen('java TestGenomicsDB -load '+loader_json_filename, shell=True,
                    stdout=subprocess.PIPE);
        elif(test_name == 'java_buffer_stream_multi_contig_t0_1_2'):
            pid = subprocess.Popen('java TestBufferStreamGenomicsDBImporter -iterators '+loader_json_filename+' '
                    +test_params_dict['stream_name_to_filename_mapping']
                    +' 1024 0 0 100 true ',
                    shell=True, stdout=subprocess.PIPE);
        elif(test_name == 'java_buffer_stream_t0_1_2'):
            pid = subprocess.Popen('java TestBufferStreamGenomicsDBImporter '+loader_json_filename
                    +' '+test_params_dict['stream_name_to_filename_mapping'],
                    shell=True, stdout=subprocess.PIPE);
        else:
            pid = subprocess.Popen(exe_path+os.path.sep+'vcf2tiledb '+loader_json_filename, shell=True,
                    stdout=subprocess.PIPE);
        stdout_string = pid.communicate()[0]
        if(pid.returncode != 0):
            sys.stderr.write('Loader test: '+test_name+' failed\n');
            cleanup_and_exit(tmpdir, -1);
        md5sum_hash_str = str(hashlib.md5(stdout_string).hexdigest())
        if('golden_output' in test_params_dict):
            golden_stdout, golden_md5sum = get_file_content_and_md5sum(test_params_dict['golden_output']);
            if(golden_md5sum != md5sum_hash_str):
                sys.stderr.write('Loader stdout mismatch for test: '+test_name+'\n');
                print_diff(golden_stdout, stdout_string);
                cleanup_and_exit(tmpdir, -1);
        if('query_params' in test_params_dict):
            for query_param_dict in test_params_dict['query_params']:
                test_query_dict = create_query_json(ws_dir, test_name, query_param_dict)
                query_types_list = [
                        ('calls','--print-calls'),
                        ('variants',''),
                        ('vcf','--produce-Broad-GVCF'),
                        ('batched_vcf','--produce-Broad-GVCF -p 128'),
                        ('java_vcf', ''),
                        ]
                for query_type,cmd_line_param in query_types_list:
                    if('golden_output' in query_param_dict and query_type in query_param_dict['golden_output']):
                        if(query_type == 'vcf' or query_type == 'batched_vcf' or query_type == 'java_vcf'):
                            test_query_dict['query_attributes'] = vcf_query_attributes_order;
                        query_json_filename = tmpdir+os.path.sep+test_name+'_'+query_type+'.json'
                        with open(query_json_filename, 'wb') as fptr:
                            json.dump(test_query_dict, fptr, indent=4, separators=(',', ': '));
                            fptr.close();
                        if(query_type == 'java_vcf'):
                            pid = subprocess.Popen('java TestGenomicsDB -query '+loader_json_filename+' '+query_json_filename,
                                    shell=True, stdout=subprocess.PIPE);
                        else:
                            pid = subprocess.Popen((exe_path+os.path.sep+'gt_mpi_gather -s %d -l '+loader_json_filename+' -j '
                                    +query_json_filename+' '+cmd_line_param)%(segment_size), shell=True,
                                    stdout=subprocess.PIPE);
                        stdout_string = pid.communicate()[0]
                        if(pid.returncode != 0):
                            sys.stderr.write('Query test: '+test_name+'-'+query_type+' failed\n');
                            cleanup_and_exit(tmpdir, -1);
                        md5sum_hash_str = str(hashlib.md5(stdout_string).hexdigest())
                        golden_stdout, golden_md5sum = get_file_content_and_md5sum(query_param_dict['golden_output'][query_type]);
                        if(golden_md5sum != md5sum_hash_str):
                            sys.stderr.write('Mismatch in query test: '+test_name+'-'+query_type+'\n');
                            print_diff(golden_stdout, stdout_string);
                            cleanup_and_exit(tmpdir, -1);
    coverage_file='coverage.info'
    subprocess.call('lcov --directory ../ --capture --output-file '+coverage_file, shell=True);
    subprocess.call("lcov --remove "+coverage_file+" '/opt*' '/usr*' 'dependencies*' -o "+coverage_file, shell=True);
    cleanup_and_exit(tmpdir, 0); 

if __name__ == '__main__':
    main()
