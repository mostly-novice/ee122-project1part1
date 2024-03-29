head	1.1;
access;
symbols;
locks; strict;
comment	@# @;


1.1
date	2009.09.19.04.51.34;	author ee122-cv;	state Exp;
branches;
next	;


desc
@this is version 8 of the testscript
@


1.1
log
@Initial revision
@
text
@#!/usr/bin/env python

_version_=0.8
print "This script is for part 1 of project 1 only."
print "Version:",_version_,"NOT used for grading!"
import sys,os,signal
if sys.version_info < (2,4):
	    print "This script requires python 2.4 or higher. Exit now."
	        sys.exit(1)

		s_bin="./server"

		try:
			    c_bin="./"+sys.argv[1]
		except:
			    c_bin="./client"

			    if not os.path.isfile(c_bin):
				        print "Can't find client binary. You should either pass its name as an argument or rename it to 'client'. Exit now."
					    sys.exit(1)

					    if not os.path.isfile(s_bin):
						        print "Can't find server binary. You should put it in the same folder. Exit now."
							    sys.exit(1)

#some util funcs
#cmd:[msgtype, length, padding], 0 means variable
dictcmd={'login':['01 ',16,2],
		        'logout':['09 ', 4,0],
			          'move':['03 ', 8,3],
				          'attack':['05 ',16,2],
					           'speak':['07 ', 0,0],
						            'north':'00 ',
							             'south':'01 ',
								               'east':'02 ',
									                 'west':'03 '}
dictmsg={'02':['Welcome to the tiny world of warcraft','A player with the same name is already in the game'],
		         '04':[': location=(', ',', '), HP=', ', EXP=',''],
			          '06':[' damaged ',' by ','. ',"'s HP is now "],
				           '08':[': '],
					            '0a':['Player ',' has left the tiny world of warcraft'],
						             '0b':['You must log in first','You already logged in']}
errmsgs=["The target is not visible",
		       "The gate to the tiny world of warcraft has disappeared",
		              ""]

def killsubprocs(subplst):
	    for p in subplst:
		            try:
				                os.kill(p.pid, signal.SIGTERM)
						        except:
								            pass

									    def cmd2hex(cmd, arg):
										        hex='04 '
											    if dictcmd[cmd][1] > 0:
												            hex+='00 %02x ' % dictcmd[cmd][1] #won't work if len>256
													        else:
															        #speak msg
																        msglen=4+len(arg)+4-len(arg)%4
																	        hex+='00 %02x ' % msglen
																		    hex+=dictcmd[cmd][0]
																		        if len(arg) > 0:
																				        try:
																						            hex+=dictcmd[arg]
																							            except:
																									                hex+="".join(['%02x ' % ord(c) for c in arg])

																											    return hex

																										    def hex2str(hex):
																											        byte=hex.split()[3:]
																												    #print byte
																												        mtype=byte[0]
																													    if mtype=='02':
																														            return dictmsg[mtype][int(byte[1])]
																														        if mtype=='04':
																																        varlst=[''.join(byte[1:byte.index('00')]).decode('hex'),
																																			                str(eval('0x'+byte[11])),
																																					                str(eval('0x'+byte[12])),
																																							                str(eval('0x'+''.join(byte[13:17]))),
																																									                str(eval('0x'+''.join(byte[17:-1])))]
																																	        return ''.join([''.join(e) for e in zip(varlst,dictmsg[mtype])])
																																	    if mtype=='06':
																																		            attacker=''.join(byte[1:byte.index('00')]).decode('hex')
																																			            tmp=byte[11:]
																																				            victim=''.join(tmp[:tmp.index('00')]).decode('hex')
																																					            damage=str(eval('0x'+byte[21]))
																																						            hp=eval('0x'+''.join(byte[22:26]))
																																							            if hp > 0:
																																									                msg=dictmsg[mtype]
																																											            return attacker+msg[0]+victim+msg[1]+damage+msg[2]+victim+msg[3]+str(hp)
																																											            else:
																																													                return attacker+' killed '+victim
																																														    if mtype=='08':
																																															            player=''.join(byte[1:byte.index('00')]).decode('hex')
																																																            tmp=byte[11:]
																																																	            msg=''.join(tmp[:tmp.index('00')]).decode('hex')
																																																		            return player+dictmsg[mtype][0]+msg
																																																		        if mtype=='0a':
																																																				        player=''.join(byte[1:byte.index('00')]).decode('hex')
																																																					        return dictmsg[mtype][0]+player+dictmsg[mtype][1]
																																																					    if mtype=='0b':
																																																						            return dictmsg[mtype][int(byte[1])]
																																																						        return 'Invalid msg type:'+mtype

#init user file so we know they will see each other
datadir='users'
player=['mario', 'peach']
pinfo=['80 0 20 20', '90 0 18 18']

if not os.path.isdir(datadir):
	    os.mkdir(datadir)

	    for i in range(len(player)):
		        f=open(datadir+'/'+player[i],'w')
			    f.write(pinfo[i])
			        f.close()

#cmds to stdin, (playerid,cmd,arg)
cmds=[(0,'login',player[0]),
		        (1,'login',player[1]),
			        (1,'move','east'),
				        (1,'move','north'),
					        (1,'login',player[1]),
						        (1,'attack',player[0]),
							        (0,'speak','I come for peace!'),
								        (1,'move','south'),
									        (1,'attack',player[0]),
										        (1,'move','west'),
											        (0,'move','west'),
												        (0,'attack',player[1]),
													        (1,'speak','wahahahahahahahah!'),
														        (0,'logout',''),
															        (1,'logout','')]

import random, fcntl, time
from subprocess import Popen, PIPE, STDOUT

#start server
port=random.randrange(30000,65000)
print "Starting server at port",port
svr=Popen([s_bin, "-p", str(port)], stdout=PIPE, stderr=STDOUT)
time.sleep(0.1)
if svr.poll() is not None:
	    print "Server error:",svr.stdout.read(),"Exit now."
	        sys.exit(1)
#get '* Listening socket is ready.\n'
		fcntl.fcntl(svr.stdout, fcntl.F_SETFL, os.O_NONBLOCK)
		time.sleep(0.2)
		svr.stdout.readline()

#start clients
print "Starting clients: ",
plst=[]
pfd={}#server side fds
for i in range(len(player)):
	    print i,
	        tmp=Popen([c_bin,'-s','127.0.0.1','-p',str(port)], stdin=PIPE, stdout=PIPE, stderr=STDOUT)
		    time.sleep(0.1)
		        if tmp.poll() is not None:
				        print "Client error:",tmp.stdout.read(),"Exit now."
					        killsubprocs(plst.append(svr))
						        sys.exit(1)
							    else:
								            plst.append(tmp)
									            #get sth like 'New connection from 127.0.0.1.53890. fd=5\n'
										            time.sleep(0.1)
											            pfd[svr.stdout.readline().split('=')[1][:-1]]=i
												    print '.'

#set client stdout to nonblock, avoid readline hang
for p in plst:
	    fcntl.fcntl(p.stdout, fcntl.F_SETFL, os.O_NONBLOCK)
	        #print 'debug if exit',p.poll()

#start testing
for cmd in cmds:
	    if cmd[0] >= len(plst):
		            print 'Client id',cmd[0],'is not valid, should be smaller than',len(plst)
			            continue
			        print "Testing client",cmd[0],':',cmd[1],cmd[2],
				    if plst[cmd[0]].poll() is not None:
					            print "Client",cmd[0],"has exited. Try next test case."
						            continue
						        plst[cmd[0]].stdin.write(cmd[1]+' '+cmd[2]+'\n')
							    goodhex=cmd2hex(cmd[1],cmd[2])
							        try:
									        time.sleep(0.1)
										        realhex=svr.stdout.readline().split('[')[1][:-1]
											        if realhex.find(goodhex) is 0:
													            print 'PASS'
														            else:
																                print 'FAIL'
																		            print 'Expect: [',goodhex,']'
																			                print 'But get:[',realhex
																					    except:
																						            print 'FAIL'
																							            print 'Expect: [',goodhex,']'
																								            print 'But get nothing.'
																									            continue

																									        while len(realhex)>0:
																											        try:
																													            svrout=svr.stdout.readline()
																														                fd=svrout.split(':')[1].split()[0]
																																            realhex=svrout.split('[')[1][:-1]
																																	                #print "Testing client",pfd[fd],"of server reply: [",realhex,
																																			            print "Testing client",pfd[fd],"of server reply",
																																				                try:
																																							                goodstr=hex2str(realhex)
																																									            except:
																																											                    import traceback
																																													                    traceback.print_exc()
																																															                try:
																																																		                time.sleep(0.1)
																																																				                realstr=plst[pfd[fd]].stdout.readline()[:-1]
																																																						                if goodstr in realstr:
																																																									                    print 'PASS'
																																																											                    else:
																																																														                        print 'FAIL'
																																																																	                    print 'Expect: ',goodstr
																																																																			                        print 'But get:',realstr.replace('command> ','')
																																																																						            except:
																																																																								                    #not always fail, may because not in sight move_notify
																																																																										                    #need func isignore, need to track all player positions
																																																																												                    #for now we make positions always in sight
																																																																														                    print 'FAIL'
																																																																																                    print 'Expect: ',goodstr
																																																																																		                    print 'But get nothing'

																																																																																				            except:
																																																																																						                break

																																																																																							print 'This script has',len(cmds),'test cases, and is NOT for grading.\nFuture versions will have more and different test cases, so a current PASS may become FAIL.'
#print svr.stdout.readlines()
																																																																																							os.kill(svr.pid, signal.SIGTERM)
																																																																																							killsubprocs(plst)
																																																																																							sys.exit(0)
																																																																																							class Msg:
																																																																																								    (LOGIN_REQUEST, LOGIN_REPLY, MOVE, MOVE_NOTIFY, ATTACK, ATTACK_NOTIFY,
																																																																																										        SPEAK, SPEAK_NOTIFY, LOGOUT, LOGOUT_NOTIFY, INVALID_STATE)=['%02d' % e for e in range(1,12)]
@
