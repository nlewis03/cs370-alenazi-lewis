
# Milestone 2 Design Document: Fridge Door & Temperature Monitor

Abdulrahman Alenazi and Nailah Lewis

## Project Overview

For our term project, we decided to build a fridge door and temperature monitor using a Raspberry Pi. I liked this idea because sometimes we forget to close the fridge door completely, especially when I am in a hurry. If the door stays open, the temperature can increase, food can spoil, and electricity can be wasted.

Our device will use two sensors. The first is a magnetic reed switch that detects whether the fridge door is open or closed. The second is a temperature sensor that measures the temperature inside the fridge.

We want our program to use both readings together. For example, opening the fridge for a few seconds is normal, but leaving it open for several minutes while the temperature increases could be a problem. We also want to check how long the fridge takes to cool down after the door closes.

The main goal is to make something useful that can monitor the fridge automatically and save information for us to review later.

## 1. Architecture

We are using a Raspberry Pi and writing our programs in C. Our plan is to use separate processes for the two sensors so they can work independently.

The door sensor will check its GPIO input every 50 milliseconds. The temperature sensor will take a reading about every 2 seconds, depending on the sensor model.

Both processes will send their readings to the storage layer.



    Door Sensor ----\
                     \
                      -- > Storage Layer
                     /            |
    Temperature ----/             v
       Sensor              Text/CLI Interface
                                      
        Supervisor
        - Monitors and restarts processes (if they die)


We also plan to make a simple text menu that lets us check the current readings, recent events, and saved information.

## 2. Mechanism Mapping

For this project, we chose mechanisms D and E from Section 3.2.

### Mechanism D - Custom Storage Layer

We want the device to save readings even when nobody is watching it. We plan to build a small append-only storage system in C.

Each record will include a timestamp, sensor type, and reading or event. We will also include record lengths and checksums to help detect damaged or incomplete records.

Instead of writing every reading separately, we plan to use a buffer and write groups of records. For important events, we will use `fsync()` to make sure the information is saved to the SD card.

If the Raspberry Pi loses power, the program will check the log during startup. It will keep complete, valid records and remove any incomplete record at the end.

This is important because we do not want a power interruption to erase everything the device recorded.

### Mechanism E - Multi-Process Architecture

Our second mechanism is using multiple processes. We chose this because I do not want one sensor problem to stop the entire device.

The door sensor and temperature sensor will run separately. A hub will receive their readings through Unix domain sockets.

We also plan to create a supervisor that checks whether the processes are still running. If a process crashes, the supervisor will detect the failure, record it, and try to restart the process.

For example, if the temperature process stops, the door sensor should continue working. The system should show that the temperature reading is unavailable instead of displaying old information as if it were current.

This design should make the device more reliable during long periods of use.

## 3. Failure-Mode Table

We know that problems can happen while the device is running, so we made a plan for detecting and handling them.

| Problem | Detection | Recovery |
|---|---|---|
| Door sensor issue | No state change over implausibly long window, or read returns invalid value | Mark status as "uncertain", keep running |
| Temperature sensor failure | Read returns error/timeout | Retry once, then mark reading unavailable |
| Sensor process crashes | Supervisor's waitpid detects exit | Supervisor restarts the process |
| Hub stops receiving data | Missing updates past expected interval | Reconnect socket or restart hub |
| Storage becomes full | fwrite/fsync returns error | Stop logging that stream, alert via interface |
| Power interruption | Startup validation finds incomplete trailing record | Truncate incomplete record, keep valid prior records |


## 4. Storage and data

Our storage system will keep information about the fridge throughout the day.

We plan to save:

- Date and time
- Sensor name and reading
- Door open or closed
- Temperature in Celsius
- Door-open duration
- Important events
- Process failures and restarts

The door sensor will check every 50 milliseconds, but we will mainly save records when the door status changes. This avoids storing the same information repeatedly.

The temperature sensor will save readings about every 2 seconds.

We plan to use an append-only binary log with record lengths and checksums. During startup, the program will check each record to make sure it is valid.

Our initial goal is to keep at least seven days of data, depending on available storage. Older records can be archived or removed when necessary.

## 5. Constraints and substitutions 

One challenge is that we are using inexpensive sensors, so their readings will not always be perfect.

One limitation of the magnetic reed switch is that a disconnected wire might look like an open door. We cannot always identify that problem from the signal alone.

The magnetic reed switch only detects whether the magnet is close enough to activate it. It cannot measure exactly how far the fridge door is open.

The temperature sensor might also take some time to respond when the door opens. Because of this, we cannot assume every temperature increase means something is wrong.

Instead, we will focus on detecting long door openings and unusually slow cooling.

Another limitation is that Nailah currently has the Raspberry Pi and both sensors. I am writing the door sensor code using VS Code on my Mac, while Nailah will connect the hardware and test the programs on the Raspberry Pi.

We will need to confirm the GPIO connections and required libraries before testing everything together.

If we use recorded or simulated data for testing, we will clearly identify it instead of presenting it as live sensor output.

## 6. Evaluation Plan

We want to test whether our device actually works instead of only checking whether the programs run.

Latency or timing: We will measure time from a door-state change (magnet near/far) to the event being written to the storage log. Our target is under 1 second, measured under both idle and loaded CPU.

Resource footprint: We will track CPU utilization and resident memory for each process (door, temp, supervisor) at steady state over the full 48 hours

Domain metric: For the door sensor, we will move the magnet close to and away from the switch 20 times and compare the results. Our target for this would be 100% detection rate (20/20). For temperature, we will compare the sensor readings with another thermometer and report the average error.

Fault injection: We will unplug the door and temp sensors seperately mid-run, and kill -9 each sensor process while running. We will also test longer door openings and observe how the temperature changes after closing the door.

Crash-Consistency (mechanism D): We will kill the program mid-write, restart, confirm log integrity.

Correlation check: We will test a scenario where the door is left open for an extended period of time and confirm that it correlates the actual door-open duration with the resulting temperature rise rather than just logging unrelated values.

## 7. Ownership Map

### My Part - Abdulrahman

I am responsible for writing a larger part of the Milestone 2 design document and developing the magnetic door sensor program in C.

I am using VS Code to write the program. It reads the GPIO input, checks whether the door is open or closed, and prints the status with a timestamp.

I also created a Makefile and instructions so Nailah can compile and test my code on the Raspberry Pi.

After the door sensor is working, I will help connect it to the main system and test it with the temperature sensor.

### Nailah's Part

I am responsible for purchasing and assembling all of the hardware, developing the temperature sensor code, editing the Milestone 2 document, and completing the demo video for M3.

I will also work on implementing the storage layer, supervisor, and initial interface/CLI. 

We will work together on further improvements to interface and final testing.

## 8. AI-Use Plan

We are using VS Code, GitHub, and C for our project. 

Disclosure: In an honest mistake, Abdulrahman initially used ChatGPT to assist in formatting an early draft of this design document, he was unaware that we needed to utilize Claude Code. That draft is not used for the final submisison.

We also plan to use Claude Code to plan architecture, review code, understand Linux system calls, draft boilerplate, and help debug problems. We will keep the required original individual Claude Code transcripts as well as document important development decisions in our `PROMPTLOG.md` files and keep both under our individual folders.

Our actual Raspberry Pi device will not depend on Claude Code, or another online AI service. The monitoring and storage programs will run locally.

## 9. Next Steps

Our next step is to get both sensors working on the Raspberry Pi and collect real readings.

Once the door sensor and temperature sensor are working, we will connect them to the main hub and storage system.

For Milestone 3, we need both sensors sending real readings through the complete pipeline into storage, with at least one menu mechanism working.

We will continue testing the device and update our design if we find problems.
