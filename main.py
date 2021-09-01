import PySimpleGUI as sg
import numpy as np
import matplotlib
import time
import os
import matplotlib.pyplot as plt
from matplotlib.widgets import Button
import serial as ser
matplotlib.use('TkAgg')

close_bool = False


def close_event(event):
    global close_bool
    close_bool = True


def reset_bool():
    global close_bool
    close_bool = False


def exit3(event):
    global close_bool
    close_bool = True


def radar_plot(r_max):
    fig = plt.figure(facecolor='k')
    win = fig.canvas.manager.window  # figure window
    screen_res = win.wm_maxsize()  # used for window formatting later
    dpi = 150.0  # figure resolution
    fig.set_dpi(dpi)  # set figure resolution
    # polar plot attributes and initial conditions
    ax = fig.add_subplot(111, polar=True, facecolor='black')
    ax.set_position([-0.2, -0.2, 1.3, 1.25])
    r_min = 2.0  # Min sensor Range
    yax = np.linspace(0.0, r_max, 5)
    ax.set_ylim([0.0, r_max])  # range of distances to show
    ax.set_xlim([0.0, np.pi])  # limited by the servo span (0-180 deg)
    ax.tick_params(axis='both', colors='g')
    ax.grid(color='green', alpha=0.7)  # grid color
    ax.set_rticks(yax)  # show 5 different distances
    ax.set_thetagrids(np.linspace(0.0, 180.0, 10))  # show 10 angles
    distance_plt = ax.text(100, r_max/5.71, 'Distance:', fontsize=10, color="green")
    angle_plt = ax.text(100, r_max/4, 'Angle:', fontsize=10, color="green")
    out_of_range = ax.text(180, r_max/6.66, '', fontsize=10, color="green")
    angles = np.arange(0, 181, 1)  # 0 - 180 degrees
    theta = angles * (np.pi / 180.0)  # to radians
    dists = np.empty((len(angles),), dtype=object)  # dummy distances until real data comes in
    pols, = ax.plot([], linestyle='', marker='o', markerfacecolor='r',
                    markeredgecolor='red', markeredgewidth=1.0,
                    markersize=2.5, alpha=0.7)  # dots for radar points
    line1, = ax.plot([], color='g',
                     linewidth=1.0)  # sweeping arm plot
    # figure presentation adjustments
    fig.set_size_inches(0.9 * (screen_res[0] / dpi), 0.9 * (screen_res[1] / dpi))
    plot_res = fig.get_window_extent().bounds  # window extent for centering
    win.wm_geometry('+{0:1.0f}+{1:1.0f}'. \
                    format((screen_res[0] / 2.0) - (plot_res[2] / 2.0),
                           (screen_res[1] / 2.0) - (plot_res[3] / 2.0)))  # centering plot
    fig.canvas.toolbar.pack_forget()  # remove toolbar for clean presentation
    fig.canvas.manager.set_window_title('MSP430 Radar')
    fig.canvas.draw()  # draw before loop
    axbackground = fig.canvas.copy_from_bbox(ax.bbox)  # background to keep during loop
    # button to close window
    close_ax = fig.add_axes([0.01, 0.01, 0.1, 0.05])
    close_but = Button(close_ax, 'Close Plot', color='#FCFCFC', hovercolor='w')
    return fig, ax, out_of_range, distance_plt, r_min, dists, angle_plt, pols, theta,\
           axbackground, line1, close_but


def main():
    s = ser.Serial(port='COM4', baudrate=9600, bytesize=ser.EIGHTBITS,
                   parity=ser.PARITY_NONE, stopbits=ser.STOPBITS_ONE,
                   timeout=80)
    s.flush()  # clear the port
    layout = [[sg.Text("\nHello to DCS Project by Roi Topaz&Yakir Peretz\n")],
              [sg.Text("Choose option from the menu:")],
              [sg.Button("1-Radar Detector System")],
              [sg.Button("2-Telemeter")],
              [sg.Button("3-Script Mode")],
              [sg.Button("Change the max distance")]]
    # Create the  GUI
    window = sg.Window("DCS Project- Roi Topaz&Yakir Peretz", layout,
                       size=(1000, 900), enable_close_attempted_event=True)
    # Load script and send them to the MCU
    script_browse = sg.popup_get_file('First script to load')
    with open(script_browse, 'r') as file:
        script1 = file.read().replace('\n', '')
    script1_name = os.path.basename(script_browse).replace('.txt', '') + '\n'
    script_browse = sg.popup_get_file('Second script to load')
    with open(script_browse, 'r') as file:
        script2 = file.read().replace('\n', '')
    script2_name = os.path.basename(script_browse).replace('.txt', '') + '\n'
    script_browse = sg.popup_get_file('Third script to load')
    with open(script_browse, 'r') as file:
        script3 = file.read().replace('\n', '')
    script3_name = os.path.basename(script_browse).replace('.txt', '') + '\n'
    script1_size = str(len(script1.encode('utf-8'))) + '\n'
    script2_size = str(len(script2.encode('utf-8'))) + '\n'
    script3_size = str(len(script3.encode('utf-8'))) + '\n'
    script1 = script1 + '\n'
    script2 = script2 + '\n'
    script3 = script3 + '\n'
    enable_tx = True
    # Send script Function - sends the name,size and the contents of the script to the MCU

    def send_script(script_name, script_size, script):
        while s.out_waiting > 0 or enable_tx:  # while the output buffer isn't empty
            bytes_name = bytes(script_name, 'ascii')
            s.write(bytes_name)
            time.sleep(0.25)
            bytes_size = bytes(script_size, 'ascii')
            s.write(bytes_size)
            time.sleep(0.25)
            bytes_script = bytes(script, 'ascii')
            s.write(bytes_script)
            time.sleep(0.25)
            if s.out_waiting == 0:
                return
    send_script(script1_name, script1_size, script1)
    send_script(script2_name, script2_size, script2)
    send_script(script3_name, script3_size, script3)
    r_max = 450.0
    #
    ############################################
    # infinite loop for checking states
    ############################################
    #
    while True:
        reset_bool()  # reset close boolean
        event0, values0 = window.read()  # read event from GUI
        if event0 == sg.WINDOW_CLOSE_ATTEMPTED_EVENT and sg.popup_yes_no('Do you really want to exit?') == 'Yes':
            break
        if event0 == "Change the max distance":
            # GUI for choose Max Dist
            layoutdist = [[sg.Text("\nChoose the Max distance for the Radar:\n")],
                            [sg.Text('Max Distance:', size=(15, 1)), sg.InputText()],
                            [sg.Submit()]]
            windowdist = sg.Window("Change Max Distance ", layoutdist, size=(600, 500))
            eventdist, valuesdist = windowdist.read()
            r_max = float(valuesdist[0])
            windowdist.close()
        if event0 == "1-Radar Detector System":
            # Send to MCU that we choose option 1
            option = '1'
            enable_tx = True
            while s.out_waiting > 0 or enable_tx:
                byte_option = bytes(option, 'ascii')
                s.write(byte_option)
                if s.out_waiting == 0:
                    enable_tx = False
            # Create the Radar plot
            fig, ax, out_of_range, distance_plt, r_min, dists, angle_plt, pols, theta,\
            axbackground, line1, close_but = radar_plot(r_max)
            close_but.on_clicked(close_event)
            fig.show()
            pols.set_data(theta, dists)
            fig.canvas.restore_region(axbackground)
            ax.draw_artist(pols)
            ax.draw_artist(distance_plt)
            ax.draw_artist(angle_plt)
            ax.draw_artist(out_of_range)
            fig.canvas.blit(ax.bbox)  # replot only data
            fig.canvas.flush_events()  # flush for next plot
            ############################################
            # inifinite loop, constantly updating the
            # 180 deg radar with incoming MSP430 data
            ############################################
            #
            while event0 == "1-Radar Detector System":
                if close_bool:  # stops program
                    plt.close(fig)
                    # Send MCU to go to sleep mode
                    option = '0'
                    enable_tx = True
                    while s.out_waiting > 0 or enable_tx:
                        byte_option = bytes(option, 'ascii')
                        s.write(byte_option)
                        if s.out_waiting == 0:
                            enable_tx = False
                    break
                read_angle = s.read_until()  # read the Angle from MSP430
                read_range = s.read_until()  # read the Range from MSP430
                decoded_angle = read_angle.decode('utf-8')  # decode data to utf-8
                decoded_range = read_range.decode('utf-8')  # decode data to utf-8
                data_angle = int((decoded_angle.replace('\n', '')))
                data_range = int((decoded_range.replace('\n', '')))
                angle = ((data_angle-400)*0.92)/10  # calc the angle
                if data_angle == 2360:
                    angle = 180.0
                dist = (data_range*34645)/2000000  # calc the Range
                out_of_range.set_text("In Range")  # set text In Range
                dist_str = "Distance:" + "{:.2f}".format(dist)
                distance_plt.set_text(dist_str)
                # check if the distance is out of range
                if dist > r_max:
                    dist = None
                    out_of_range.set_text("Out of Range")
                    distance_plt.set_text("Distance:")
                elif dist < r_min:
                    dist = None
                    out_of_range.set_text("Out of Range")
                    distance_plt.set_text("Distance:")
                # Update the data in the Radar plot
                dists[int(angle)] = dist
                angle_plt.set_text("Angle:" + "{:.2f}".format(angle))
                pols.set_data(theta, dists)
                fig.canvas.restore_region(axbackground)
                ax.draw_artist(pols)
                line1.set_data(np.repeat((angle * (np.pi / 180.0)), 2),
                               np.linspace(0.0, r_max, 2))
                ax.draw_artist(line1)
                ax.draw_artist(distance_plt)
                ax.draw_artist(angle_plt)
                ax.draw_artist(out_of_range)
                fig.canvas.blit(ax.bbox)  # replot only data
                fig.canvas.flush_events()  # flush for next plot
        if event0 == "2-Telemeter":
            # send MCU that we are in state 2
            option = '2'
            enable_tx = True
            while s.out_waiting > 0 or enable_tx:
                byte_option = bytes(option, 'ascii')
                s.write(byte_option)
                if s.out_waiting == 0:
                    enable_tx = False
            # GUI for choose angle
            layout2 = [[sg.Text("\nYou choose option 2 in the main menu. Insert the Angle:\n")],
                       [sg.Text('Angle:', size=(15, 1)), sg.InputText()],
                       [sg.Submit()]]
            window2 = sg.Window("Option 2 ", layout2, size=(600, 500))
            event2, values2 = window2.read()
            inputangle = values2[0] + '\n'
            angle = float(values2[0])
            window2.close()
            # send chosen angle to MCU
            enable_tx = True
            while s.out_waiting > 0 or enable_tx:
                byte_angle = bytes(inputangle, 'ascii')
                s.write(byte_angle)
                if s.out_waiting == 0:
                    enable_tx = False
            # create the Radar plot
            fig, ax, out_of_range, distance_plt, r_min, dists, angle_plt, pols, theta,\
            axbackground, line1, close_but = radar_plot(r_max)
            close_but.on_clicked(close_event)
            fig.show()
            pols.set_data(theta, dists)
            fig.canvas.restore_region(axbackground)
            ax.draw_artist(pols)
            ax.draw_artist(distance_plt)
            ax.draw_artist(angle_plt)
            ax.draw_artist(out_of_range)
            fig.canvas.blit(ax.bbox)  # replot only data
            fig.canvas.flush_events()  # flush for next plot
            while event0 == "2-Telemeter":
                if close_bool:  # stops program
                    plt.close(fig)
                    # send MCU to change state to sleep mode
                    option = '0'
                    enable_tx = True
                    while s.out_waiting > 0 or enable_tx:
                        byte_option = bytes(option, 'ascii')
                        s.write(byte_option)
                        if s.out_waiting == 0:
                            enable_tx = False
                    break
                # read Range from MCU
                read_range = s.read_until()
                decoded_range = read_range.decode('utf-8')  # decode data to utf-8
                data_range = int((decoded_range.replace('\n', '')))
                dist = (data_range * 34645) / 2000000  # calc the Range
                out_of_range.set_text("In Range")  # set text In range
                dist_str = "Distance:" + "{:.2f}".format(dist)
                distance_plt.set_text(dist_str)
                # check if the distance is out of range
                if dist > r_max:
                    dist = None
                    out_of_range.set_text("Out of Range")
                    distance_plt.set_text("Distance:")
                elif dist < r_min:
                    dist = None
                    out_of_range.set_text("Out of Range")
                    distance_plt.set_text("Distance:")
                # update the data in the Radar plot
                dists[int(angle)] = dist
                angle_plt.set_text("Angle:" + "{:.2f}".format(angle))
                pols.set_data(theta, dists)
                fig.canvas.restore_region(axbackground)
                ax.draw_artist(pols)
                line1.set_data(np.repeat((angle * (np.pi / 180.0)), 2),
                               np.linspace(0.0, r_max, 2))
                ax.draw_artist(line1)
                ax.draw_artist(distance_plt)
                ax.draw_artist(angle_plt)
                ax.draw_artist(out_of_range)
                fig.canvas.blit(ax.bbox)  # replot only data
                fig.canvas.flush_events()  # flush for next plot
        if event0 == "3-Script Mode":
            # send to MCU that we are in State 3
            option = '3'
            enable_tx = True
            while s.out_waiting > 0 or enable_tx:
                byte_option = bytes(option, 'ascii')
                s.write(byte_option)
                if s.out_waiting == 0:
                    enable_tx = False
            # choose file to run and check the name of the file
            fname = sg.popup_get_file('Script to run')
            fname = os.path.basename(fname).replace('.txt', '') + '\n'
            # send the script name to the MCU
            enable_tx = True
            while s.out_waiting > 0 or enable_tx:
                byte_sendscript = bytes(fname, 'ascii')
                s.write(byte_sendscript)
                if s.out_waiting == 0:
                    enable_tx = False
            fig3 = plt.figure(facecolor='k')
            ax3 = fig3.add_subplot(111, facecolor='black')
            text_script = ax3.text(0, 0, 'Your script is running...\n', fontsize=15, color="white")
            close_ax3 = fig3.add_axes([0.2, 0.01, 0.5, 0.1])
            close_but3 = Button(close_ax3, 'Exit Script Mode', color='#FCFCFC', hovercolor='w')
            fig3.canvas.toolbar.pack_forget()  # remove toolbar for clean presentation
            fig3.canvas.manager.set_window_title('MSP430 Radar')
            fig3.canvas.draw()  # draw before loop
            axbackground3 = fig3.canvas.copy_from_bbox(ax3.bbox)  # background to keep during loop
            close_but3.on_clicked(exit3)
            fig3.show()
            while event0 == "3-Script Mode":
                read_ack_ch = 'H'
                fig3.canvas.blit(ax3.bbox)  # replot only data
                fig3.canvas.flush_events()  # flush for next plot
                if close_bool:  # stops program
                    plt.close(fig3)
                    # Send MCU to go to sleep mode
                    option = '0'
                    enable_tx = True
                    while s.out_waiting > 0 or enable_tx:
                        byte_option = bytes(option, 'ascii')
                        s.write(byte_option)
                        if s.out_waiting == 0:
                            enable_tx = False
                    break
                if s.inWaiting() > 0:
                    read_ack = s.read(size=1)  # read ack from MCU
                    read_ack_ch = read_ack.decode("ascii")
                if read_ack_ch == 'Q':  # ack for opcode 6
                    # create the Radar plot
                    fig, ax, out_of_range, distance_plt, r_min, dists, angle_plt, pols, theta, axbackground,\
                    line1, close_but = radar_plot(r_max)
                    fig.show()
                    pols.set_data(theta, dists)
                    fig.canvas.restore_region(axbackground)
                    ax.draw_artist(pols)
                    ax.draw_artist(distance_plt)
                    ax.draw_artist(angle_plt)
                    ax.draw_artist(out_of_range)
                    fig.canvas.blit(ax.bbox)  # replot only data
                    fig.canvas.flush_events()  # flush for next plot
                    read_angle = s.read_until()  # read the Angle from MSP430
                    read_range = s.read_until()  # read the Range from MSP430
                    decoded_angle = read_angle.decode('utf-8')  # decode data to utf-8
                    decoded_range = read_range.decode('utf-8')  # decode data to utf-8
                    data_angle = int((decoded_angle.replace('\n', '')))
                    data_range = int((decoded_range.replace('\n', '')))
                    angle = int((((data_angle - 400) * 0.92) / 10)+0.92)  # calc the angle
                    dist = (data_range * 34645) / 2000000  # calc the Range
                    out_of_range.set_text("In Range")
                    dist_str = "Distance:" + "{:.2f}".format(dist)
                    distance_plt.set_text(dist_str)
                    # check if the distance measured is in range
                    if dist > r_max:
                        dist = None
                        out_of_range.set_text("Out of Range")
                        distance_plt.set_text("Distance:")
                    elif dist < r_min:
                        dist = None
                        out_of_range.set_text("Out of Range")
                        distance_plt.set_text("Distance:")
                    # draw the data in the Radar plot
                    dists[int(angle)] = dist
                    angle_plt.set_text("Angle:" + "{:.2f}".format(angle))
                    pols.set_data(theta, dists)
                    fig.canvas.restore_region(axbackground)
                    ax.draw_artist(pols)
                    line1.set_data(np.repeat((angle * (np.pi / 180.0)), 2),
                                   np.linspace(0.0, r_max, 2))
                    ax.draw_artist(line1)
                    ax.draw_artist(distance_plt)
                    ax.draw_artist(angle_plt)
                    ax.draw_artist(out_of_range)
                    fig.canvas.blit(ax.bbox)  # replot only data
                    fig.canvas.flush_events()  # flush for next plot
                    time.sleep(2)  # delay, so we can see the Radar
                    plt.close()   # close the Radar plot
                elif read_ack_ch == 'W':  # ack for opcode 7
                    read_fangle = s.read_until()  # read the Final Angle from MSP430
                    decoded_fangle = read_fangle.decode('ascii')  # decode data to utf-8
                    finalangle = int(decoded_fangle, 16)  # convert angle from hex ti integer
                    # create the Radar plot
                    fig, ax, out_of_range, distance_plt, r_min, dists, angle_plt, pols, theta, axbackground,\
                    line1, close_but = radar_plot(r_max)
                    close_but.on_clicked(close_event)
                    fig.show()
                    pols.set_data(theta, dists)
                    fig.canvas.restore_region(axbackground)
                    ax.draw_artist(pols)
                    ax.draw_artist(distance_plt)
                    ax.draw_artist(angle_plt)
                    ax.draw_artist(out_of_range)
                    fig.canvas.blit(ax.bbox)  # replot only data
                    fig.canvas.flush_events()  # flush for next plot
                    while True:  # read and draw angle and distance until the Final angle
                        if close_bool:  # stops program
                            plt.close(fig)
                            break
                        read_angle = s.read_until()  # read the Angle from MSP430
                        read_range = s.read_until()  # read the Range from MSP430
                        decoded_angle = read_angle.decode('utf-8')  # decode data to utf-8
                        decoded_range = read_range.decode('utf-8')  # decode data to utf-8
                        data_angle = int((decoded_angle.replace('\n', '')))
                        data_range = int((decoded_range.replace('\n', '')))
                        angle = ((data_angle - 400) * 0.92) / 10  # calc angle
                        if angle + 0.92 > finalangle:  # check if angle if the final angle
                            angle = finalangle
                        dist = (data_range * 34645) / 2000000  # calc Range
                        out_of_range.set_text("In Range")
                        dist_str = "Distance:" + "{:.2f}".format(dist)
                        distance_plt.set_text(dist_str)
                        # check if the distance measured is out of range
                        if dist > r_max:
                            dist = None
                            out_of_range.set_text("Out of Range")
                            distance_plt.set_text("Distance:")
                        elif dist < r_min:
                            dist = None
                            out_of_range.set_text("Out of Range")
                            distance_plt.set_text("Distance:")
                        dists[int(angle)] = dist
                        angle_plt.set_text("Angle:" + "{:.2f}".format(angle))
                        pols.set_data(theta, dists)
                        fig.canvas.restore_region(axbackground)
                        ax.draw_artist(pols)
                        line1.set_data(np.repeat((angle * (np.pi / 180.0)), 2),
                                       np.linspace(0.0, r_max, 2))
                        ax.draw_artist(line1)
                        ax.draw_artist(distance_plt)
                        ax.draw_artist(angle_plt)
                        ax.draw_artist(out_of_range)
                        fig.canvas.blit(ax.bbox)  # replot only data
                        fig.canvas.flush_events()  # flush for next plot
                        if angle == finalangle:  # if we reached the final angle - close the Radar plot
                            plt.close(fig)
                            break
                elif read_ack_ch == 'C':  # ack from the MCU that we are no longer at state 3
                    plt.close(fig3)
                    break

    window.close()


if __name__ == '__main__':
    main()
