# We are going to keep a list of players that are active

# Login_request

# First you check if the guy is login by searching through the active clients
handleLoginRequest
if the guy is already logged in:
    send a invalid state
else:
    make a new instance of Player
    check if the file with that name exists
    if name does not exist:
        open a new file
        randomize stats
        write stats to the file
        initialize the player
    else if name exists:
        open an existed file
        read from the file and initialize the player
    send LOGIN_REPLY to the player
    send MOVE_NOTIFY to the rest of the players

# Do we need to write to server every 

def handleMove(player,direction):
    if direction == NORTH:
        p.y += 3
    elif direction == SOUTH:
        p.y -= 3
    elif direction == WEST:
        p.x -= 3
    elif direction == EAST:
        p.x += 3

    # Either write to this here or when the server exits
    write to file here
    
    broadcast MOVE_NOTIFY

def handleAttack(attacker,victim):
    if the attacker is the same as the victim:
        ignore this message
    if there is no such victim:
        ignore this message
    damage = random(10,20)
    if damage > victim.hp:
        damage = victim.hp

    victim.hp -= damage
    attack.exp += damage

    broadcast ATTACK_NOTIFY with attacker.name,victim.name,damage,victim.hp

    if victim.hp == 0:
        victim.hp = random(30-50);
        vicctim.x = random(0,99);
        victim.y = random(0,99);
    
def handleSpeak(player,message):
    broadcast(SPEAK_NOTIFY,name,message);

def handleLogout(player);
	store data for player
	disconnect player
	broadcast logout_notify

handleLoginReply
Form the packets and send them
