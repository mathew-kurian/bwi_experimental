#!/usr/bin/env python

from bwi_rlg.srv import *
import rospy
import time
import subprocess
import os.path
import os
import glob

def handle_semantic_parser(req):

    if req.type == 0: # QUESTION_ASKING

        rospy.loginfo("Human: " + req.input_text)

        path_to_bwi_rlg = rospy.get_param("/semantic_parser_server/path_to_bwi_rlg")

        print("path_to_bwi_rlg: " + path_to_bwi_rlg)

        file_last_comm = path_to_bwi_rlg + "/agent/dialog/last_comm_time.txt"
        time_curr = time.time()
        
        print("file_last_comm: " + file_last_comm)

        if os.path.exists(file_last_comm):

            f = open(file_last_comm, 'r')
            time_last = f.readline()
            time_last = time_last.replace("\n", "")
            id_last = f.readline()
            id_last = id_last.replace("\n", "")
            f.close()
            print("time_curr: " + str(time_curr))
            print("time_last: " + str(time_last))
            time_diff = float(time_curr) - float(time_last)

        else:

            id_last = "id_" + str(time.time())
            time_diff = 0

        path_to_main = path_to_bwi_rlg + "/agent/dialog/"

        filelist = glob.glob(path_to_main + "offline_data/outputs/*")
        for f in filelist:
            os.remove(f)

        print(str(time_diff))


        if time_diff < rospy.get_param("patience_time_in_conversation"):

            f = open(path_to_main + "offline_data/inputs/" + id_last + \
                    "_input.txt", 'w')
            f.write(req.input_text)
            f.close()
            
            cmd = "python " + path_to_main + "main.py " + path_to_main + \
                    "/ offline " + id_last
            subprocess.Popen(cmd, shell=True)

            output_file = path_to_main + "offline_data/outputs/" + id_last + \
                "_output.txt"

            while True:
                if os.path.exists(output_file):
                    diff = abs(time.time() - os.path.getmtime(output_file))
                    if diff < 1:
                        break
                else:
                    time.sleep(0.1)

            while os.path.exists(output_file) == False:
                time.sleep(0.1)

            f = open(output_file, 'r')
            output = f.read()
            f.close()

            f = open(file_last_comm, 'w+')
            f.write(str(time.time()) + '\n')
            f.write(id_last)
            f.close()

            id_curr = id_last

        else: 

            id_new = "id_" + str(time.time())
            
            f = open(path_to_main + "offline_data/inputs/" + id_new + \
                    "_input.txt", 'w+')
            f.write(req.input_text)
            f.close()

            cmd = "python " + path_to_main + "main.py " + path_to_main + \
                    "/ offline " + id_new
            subprocess.Popen(cmd, shell=True)

            output_file = path_to_main + "offline_data/outputs/" + id_new + \
                "_output.txt"

            while os.path.exists(output_file) == False:
                time.sleep(0.1)

            f = open(output_file, 'r')
            output = f.read()
            f.close()

            f = open(file_last_comm, 'w+')
            f.write(str(time.time()) + '\n')
            f.write(id_new)
            f.close()
           
            id_curr = id_new

        path_to_command = path_to_main + "offline_data/commands/" + id_curr + \
                          "_command.txt"
        if os.path.exists(path_to_command):
            f = open(path_to_command, 'r')
            command = f.readline()
            command = command.replace("\n", "")
            print("path_to_command: " + command)
        else:
            command = ""
            

        return SemanticParserResponse(str(output), command)

    elif req.type == 1: # TRAINING
        
        res = "Training: not implemented."
        rospy.loginfo(res)
        return SemanticParserResponse(res)

    elif req.type == 2: # STARTOVER

        res = "Start over: not implemnted."
        rospy.loginfo(res)
        return SemanticParserResponse(res)

    else:
        rospy.logerr("Error in semantic_parser_server.")

def semantic_parser_server():

    rospy.init_node('semantic_parser_server')
    s = rospy.Service('semantic_parser', SemanticParser, handle_semantic_parser)
    rospy.spin()


if __name__ == "__main__":

    # rospy.set_param("path_to_bwi_rlg", "/home/szhang/catkin_ws/src/bwi_experimental/bwi_rlg/")
    rospy.set_param("patience_time_in_conversation", 30)
    semantic_parser_server()   

