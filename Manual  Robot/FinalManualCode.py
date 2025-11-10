# Upload in mBlock
import gamepad
from cyberpi import *
import time

# show to start
console.println('START EiEi')

# robot speed
DRIVE_SPEED = 100

motors_on = True
prev_1 = False

# Loop for Movement
while True:
    if gamepad.is_key_pressed('Right'):
        mbot2.drive_power(DRIVE_SPEED, DRIVE_SPEED)
        
    elif gamepad.is_key_pressed('Left'):
        mbot2.drive_power(-DRIVE_SPEED, -DRIVE_SPEED)
        
    elif gamepad.is_key_pressed('Up'):
        mbot2.drive_power(DRIVE_SPEED, -DRIVE_SPEED)
        
    elif gamepad.is_key_pressed('Down'):
        mbot2.drive_power(-DRIVE_SPEED, DRIVE_SPEED)
        
    else:
        mbot2.drive_power(0, 0)
    
    # use pressed 1 to start/stop DCmotor for shopt
    is_1_pressed = gamepad.is_key_pressed('1')
    
    if is_1_pressed and not prev_1:
        motors_on = not motors_on
    prev_1 = is_1_pressed
    
    if motors_on:
        mbot2.motor_set(-100, "M3")
        mbot2.motor_set(100, "M4")
    else:
        mbot2.motor_stop("M3")
        mbot2.motor_stop("M4")
        
    time.sleep(0.05)