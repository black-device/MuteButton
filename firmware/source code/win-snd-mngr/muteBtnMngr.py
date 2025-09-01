#!/usr/bin/env python3

from collections.abc import Callable, Iterable, Mapping
import platform
import threading
import time
import serial
from serial.tools import list_ports
import sys
import glob
import typing_extensions
import queue
import math
from debug import dbgPrint


#import sounddevice as sd
if platform.system() == 'Windows':
    from comtypes import CLSCTX_ALL
    import pythoncom
    from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume
    from pycaw.constants import (DEVICE_STATE, EDataFlow,)



class AudioStuff():
    def __init__(self) -> None:
        self._getAudioDevices()

        foo = AudioUtilities.GetAllDevices()

        for d in foo: dbgPrint(f'AudioStuff---> [{d}]: [{AudioUtilities.GetEndpointDataFlow(d.id)}]')

        pythoncom.CoInitialize()
        self._getInputDevices()

             
    def _getInputDevices(self):
        ret = []
        devEnum = AudioUtilities.GetDeviceEnumerator()
        if devEnum is None: return ret

        inDevsCollection = devEnum.EnumAudioEndpoints(EDataFlow.eCapture.value, DEVICE_STATE.MASK_ALL.value)
        if inDevsCollection is None: return ret

        cnt = inDevsCollection.GetCount()

        for i in range(cnt):
            d = inDevsCollection.Item(i)
            if d is None: continue
            dbgPrint(f'AudioStuff. _getInputDevices---> [{d.GetId()}; {d.GetState()}; ]')
            if d.GetState() == 1: ret.append(d)

        return ret
    

    def _muteAllInputs(self, muted: bool):
        dbgPrint(f'AudioStuff._muteAllInputs. Setting Mute: {muted}')
        pythoncom.CoInitialize()
        devs = self._getInputDevices()

        for d in devs: 
            try:
                dbgPrint(f'AudioStuff. _muteAllInputs ---> [{d}]')
                i = d.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
                vInHdlr = i.QueryInterface(IAudioEndpointVolume)
                vInHdlr.SetMute(muted, None)
            except Exception as ex:
                dbgPrint(f'AudioStuff. _muteAllInputs. Error try to mute [{d}]: {ex}')


    def _getAudioDevices(self):
        self.outputDeviceAvailable = False
        self.inputDeviceAvailable = False

        # obtain output devices
        try:
            pythoncom.CoInitialize()
            devices = AudioUtilities.GetSpeakers()
            interface = devices.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
            self.volOutHandler = interface.QueryInterface(IAudioEndpointVolume)
            self.outputDeviceAvailable = True
        except Exception as ex:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            dbgPrint(f'AudioStuff._getAudioDevices. Error obtaining output device: {ex}\r\n'
                     f'    {exc_type}\r\n'
                     f'    {exc_obj}\r\n'
                     f'    In [{exc_tb.tb_frame.f_code.co_filename}], line: {exc_tb.tb_lineno}')

        # obtain input devices
        try:
            pythoncom.CoInitialize()
            devices = AudioUtilities.GetMicrophone()
            interface = devices.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
            self.volInHandler = interface.QueryInterface(IAudioEndpointVolume)
            self.inputDeviceAvailable = True
        except Exception as ex:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            dbgPrint(f'AudioStuff._getAudioDevices. Error obtaining input device: {ex}\r\n'
                     f'    {exc_type}\r\n'
                     f'    {exc_obj}\r\n'
                     f'    In [{exc_tb.tb_frame.f_code.co_filename}], line: {exc_tb.tb_lineno}')


    def getMute(self) -> bool:
        dbgPrint(f'AudioStuff.getMute. Getting mute...')
        if not self.inputDeviceAvailable: 
            self._getAudioDevices()
            if not self.inputDeviceAvailable:
                dbgPrint(f'AudioStuff.getMute. No input device available')
                return None
        
        try:
            mute = self.volInHandler.GetMute() > 0
            dbgPrint(f'AudioStuff.getMute. Mute: {mute}')
            return mute
        except Exception as ex:
            self.inputDeviceAvailable = False
            dbgPrint(f'AudioStuff.getMute. Error trying to get muted state: {ex}')
            return None
            


    def setMute(self, muted: bool) -> bool:
        dbgPrint(f'AudioStuff.setMute. Setting Mute: {muted}')
        if not self.inputDeviceAvailable: 
            self._getAudioDevices()
            if not self.inputDeviceAvailable:
                dbgPrint(f'AudioStuff.setMute. No input device available')
                return False
            
        try:
            #self.volInHandler.SetMute(muted, None)
            self._muteAllInputs(muted)
            return True
        except Exception as ex:
            self.inputDeviceAvailable = False
            dbgPrint(f'AudioStuff.setMute. Error trying to set mute state: {ex}')
            return False
        

    def getVolume(self) -> int:
        dbgPrint(f'AudioStuff.getVolume. Getting volume...')
        if not self.outputDeviceAvailable: 
            self._getAudioDevices()
            if not self.outputDeviceAvailable:
                dbgPrint(f'AudioStuff.getVolume. No output device available')
                return None
            
        try:
            vMin, vMax, v0 = self.volOutHandler.GetVolumeRange()
            rawVol = self.volOutHandler.GetMasterVolumeLevel()
            vol = int(AudioStuff._dB2Percent(rawVol, vMin, vMax))
            dbgPrint(f'AudioStuff.getVolume. Raw volume [{vMin}, {vMax}, {v0}]: [{rawVol} dB] -> [{vol}%]')
            # u = (vMin - vMax)
            # vol = int(math.pow(10, (rawVol/34))*100)
            # dbgPrint(f'AudioStuff.getVolume. Volume: {vol}')
            return vol
        except Exception as ex:
            self.outputDeviceAvailable = False
            dbgPrint(f'AudioStuff.getVolume. Error trying to get output volume value: {ex}')
            return None
        

    def setVolume(self, vol: int) -> bool:
        dbgPrint(f'AudioStuff.setVolume. Setting Volume: {vol}')
        if not self.outputDeviceAvailable: 
            self._getAudioDevices()
            if not self.outputDeviceAvailable:
                dbgPrint(f'AudioStuff.setVolume. No output device available')
                return False
            
        try:
            vMin, vMax, v0 = self.volOutHandler.GetVolumeRange()
            rawVol = AudioStuff._percent2dB(vol, vMin, vMax)
            dbgPrint(f'AudioStuff.setVolume. Raw volume [{vMin}, {vMax}, {v0}]: [{vol} %] -> [{rawVol} dB]')
            self.volOutHandler.SetMasterVolumeLevel(rawVol, None)
            return True
        except Exception as ex:
            self.outputDeviceAvailable = False
            dbgPrint(f'AudioStuff.setVolume. Error trying to set volume: {ex}')
            return False
        
    
    def _dB2Percent(dB: float, dBMin: float, dBMax: float) -> float:
        if dB <= dBMin: dB = dBMin + 0.0000001
        if dB > dBMax: dB = dBMax

        dbRange = (dBMax - dBMin)
        scale = 100.0 * math.pow(10, ((dB - dBMax)/(dbRange * 0.45)))
        return max(0, min(scale, 100)) 
    

    def _percent2dB(percent: float, dBMin: float, dBMax: float) -> float:
        if percent <= 0.0: percent = 0.0000000001
        if percent > 100.0: percent = 100.0

        dbRange = (dBMax - dBMin)
        dB = math.log10(percent/100.0)*dbRange*0.45 + dBMax
    
        return max(dBMin, min(dB, dBMax))
    
    def scale_audio_level(dB, dB_min, dB_max):
        if dB < dB_min or dB > dB_max:
            raise ValueError("El nivel de dB debe estar entre dB_min y dB_max")

        # Escalamos linealmente de dB a un rango de 0 a 1
        linear_scale = (dB - dB_min) / (dB_max - dB_min)

        # Aplicamos una transformación logarítmica para una escala perceptual
        if linear_scale <= 0:
            return 0
        scale = 100 * (math.log10(linear_scale + 1) / math.log10(2))
        return max(0, min(scale, 100))
    

    def unscale_audio_level(scaled_value, dB_min, dB_max):
        if scaled_value < 0 or scaled_value > 100:
            raise ValueError("El valor escalado debe estar entre 0 y 100")

        # Convertimos de la escala de 0 a 100 a la escala de dB_min a dB_max
        dB = dB_min + (math.sqrt(scaled_value / 100) * (dB_max - dB_min))
        return max(dB_min, min(dB, dB_max))

#______________________________________________________________________________
#______________________________________________________________________________
#______________________________________________________________________________


class MuteButtonDevice(threading.Thread):
    MsgMuteOn = 1
    MsgMuteOff = 2
    MsgVolumeUp = 3
    MsgVolumeDown = 4

    StateMuteOff = 0
    StateMuteOn = 1
    StateError = 2
    

    def __init__(self, serialPort: serial.Serial):
        threading.Thread.__init__(self)
        self.serialPort = serialPort
        self.q = queue.Queue()
        self._timeout = 0.1
        self._isAlive = True
        self.__lock = threading.Lock()
        self.__lkcnt = 0
        self._keepRunning = True
        self._keepAliveTmr = 0
        self._currState = 0
        self._currVolume = 0


    def __del__(self):
        dbgPrint(f'MuteButtonDevice. Destructor...')
        self._keepRunning = False


    def run(self):
        dbgPrint(f'MuteButtonDevice. Task start...')
        self._keepAliveTmr = time.time()
        while self._keepRunning:
            self._muteButtonDevRead()
            # keep alive
            if (time.time() - self._keepAliveTmr) > 1.5: 
                self._keepAliveTmr = time.time()
                self._sendState()


    def join(self, timeout: float = None) -> None:
        self._keepRunning = False
        return super().join(timeout)


    def getMessage(self) -> int:
        ret = None
        if self.q.qsize() > 0: 
            ret = self.q.get()
            dbgPrint(f'____Queue OUT: [ {self.q.qsize()} ] _______________________')
        return ret


    def configureColor(self, colorList: list[int]):
        data = []
        for e in colorList:
            rmu = e & 0xff
            gmu = (e>>8) & 0xff
            bmu = (e>>16) & 0xff
            data += [rmu, gmu, bmu]
        try:
            self._lock()
            self.serialPort.write(data)
            self._unlock()
            time.sleep(0.02)
        except Exception as e:
            self._unlock()
            dbgPrint(f'MuteButtonDevice.configureColor. Exception writing serial port: {e}')
        dbgPrint(f'MuteButtonDevice.configcolor. exit')


    def setState(self, state: int, vol: int = 0):
        dbgPrint(f'MuteButtonDevice.setState. State: {state}; volume: {vol}')
        self._currState = state
        self._currVolume = int(vol)
        self._keepAliveTmr = 0


    def _lock(self):
        self.__lkcnt += 1
        localLckCnt = self.__lkcnt
        wasLocked = (localLckCnt > 1)
        if  wasLocked: dbgPrint(f'MuteButtonDevice._lock IS LOCKED ({localLckCnt})')
        while self.__lkcnt > localLckCnt: time.sleep(0.01)
        if  wasLocked: dbgPrint(f'MuteButtonDevice._lock LOCKED free ({self.__lkcnt})')


    def _unlock(self):
        if self.__lkcnt > 0: self.__lkcnt -= 1
        return
        self.__lock.release()
        self.__lkcnt -= 1


    def _sendState(self):
        dbgPrint(f'MuteButtonDevice._sendState. State: {self._currState}; volume: {self._currVolume}')
        self._keepAliveTmr = time.time()
        try:
            self._lock()
            self.serialPort.write([(self._currState & 0xff), (self._currVolume & 0xff)])
            time.sleep(0.02)
            self._unlock()
        except Exception as e:
            self._unlock()
            self._isAlive = False
            dbgPrint(f'MuteButtonDevice._sendState. Exception writing serial port: {e}')
        dbgPrint(f'MuteButtonDevice._sendState. exit')


    def _muteButtonDevRead(self) -> bool:
        res = ''
        try:
            self._lock()
            self.serialPort.timeout = 0.1
            res = self.serialPort.readall()
            self._unlock()
        except Exception as e:
            self._unlock()
            dbgPrint(f'MuteButtonDevice._muteButtonDevRead. Exception reading serial port: {e}')
            time.sleep(0.1)
            self._isAlive = False
            return False
        
        if len(res) == 0: 
            self._timeout = 0.1
            return False
        
        dbgPrint(f'MuteButtonDevice._muteButtonDevRead. Received {len(res)} bytes')
        self._keepAliveTmr = time.time()
        
        for c in res:
            dbgPrint(f'MuteButtonDevice._muteButtonDevRead. Order received from device: {c}')
            if self.q is None: return True
            if c == 0xf1: self.q.put(MuteButtonDevice.MsgMuteOff)
            elif c == 0xf2: self.q.put(MuteButtonDevice.MsgMuteOn)
            elif c == 0xf8: self.q.put(MuteButtonDevice.MsgVolumeUp)
            elif c == 0xf9: self.q.put(MuteButtonDevice.MsgVolumeDown)

            dbgPrint(f'____Queue IN: [ {self.q.qsize()} ] ______________')
            time.sleep(0.05)
        
        return True
    

    def isAlive(self) -> bool: return self._isAlive
    

    def findMuteButtonDevice():# -> typing_extensions.Self:
        ret: serial.Serial

        for p in MuteButtonDevice._getMuteButtonPorts():
            dbgPrint(f'MuteButtonDevice.findMuteButtonDevice. Checking port [{p}]...')
            try:
                ret = serial.Serial(p, 115200, timeout=0.1)
                if MuteButtonDevice._pingPort(ret): return MuteButtonDevice(ret)
            except Exception as ex:
                dbgPrint(f'MuteButtonDevice.findMuteButtonDevice. Error opening port [{p}]: {ex}')
        return None


    def _pingPort(port: serial.Serial) -> bool:
        dbgPrint(f'MuteButtonDevice._pingPort')
        try:
            port.timeout = 0.05
            res = port.readall()
            port.timeout = 0.1
            port.write([0x11, 0x22, 0x33, 0x44])
            res = port.readall()
        
            return len(res) == 1 and res[0] == 0x55
        except Exception as e:
            dbgPrint(f'MuteButtonDevice._pingPort. Exception reading serial port: {e}')
            return False


    def _getMuteButtonPorts() -> [str]:
        devList = list_ports.comports()
        for d in devList:
            if d.vid is None or d.pid is None: continue
            dbgPrint(f'_getAvalSerialPorts: {d.vid:04X}:{d.pid:04X}')
            if f'{d.vid:04X}' == '1209' and f'{d.pid:04X}' == '48AF':
                return [d.device]
        return []
            


#______________________________________________________________________________
#______________________________________________________________________________
    

class App(threading.Thread):

    def __init__(self, event):
        threading.Thread.__init__(self)
        dbgPrint(f'App. Init...')
        self.event = event
        self._keepRunning = True
        self.audio = AudioStuff()
        self.currentState = 0xff
        self.lastState = self.currentState - 1
        self.isMutedLastRead = 0

        self.currentVolume = 0
        self.lastVolume = 1
        self.currentVolumeLastRead = 0

        self.dev = self.setDevice(self.currentState, self.currentVolume)
        self._externalCallState: int = None
        self._externalCallVolume: int = None


    def run(self) -> None:
        dbgPrint(f'App. Init task...')

        while self._keepRunning:
            time.sleep(0.05)
            currTime = time.time() 

            if not self.deviceOnline(self.dev):
                dbgPrint(f'App.run. Device not attached...............')
                if self.event is not None: self.event(MuteButtonDevice.StateError, int(0))
                if self.dev is not None: self.dev.join()
                self.dev = self.setDevice(self.currentState, self.currentVolume)
                if self.event is not None: self.event(self.currentState, int(self.currentVolume))
                continue

            while True:
                time.sleep(0.01)
                msg = self.dev.getMessage()
                if msg is None: break
                if not self.dev.isAlive(): break
                
                if msg == MuteButtonDevice.MsgMuteOn: 
                    if not self.audio.setMute(True): self.currentState = MuteButtonDevice.StateError
                    self.isMutedLastRead = 0
                elif msg == MuteButtonDevice.MsgMuteOff: 
                    if not self.audio.setMute(False): self.currentState = MuteButtonDevice.StateError
                    self.isMutedLastRead = 0
                elif msg == MuteButtonDevice.MsgVolumeUp:
                    self.currentVolume += 5.0
                    if self.currentVolume > 100: self.currentVolume = 100.0
                elif msg == MuteButtonDevice.MsgVolumeDown:
                    self.currentVolume -= 5
                    if self.currentVolume < 0: self.currentVolume = 0.0

                if self.lastVolume != self.currentVolume and self.currentState != MuteButtonDevice.StateError: 
                    if not self.audio.setVolume(self.currentVolume): self.currentState = MuteButtonDevice.StateError

            if self._externalCallState is not None:
                dbgPrint(f'App.run. External change. State [{self._externalCallState}]')
                if self._externalCallState == MuteButtonDevice.StateMuteOff:
                    if not self.audio.setMute(False): self.currentState = MuteButtonDevice.StateError
                    self.isMutedLastRead = 0
                if self._externalCallState == MuteButtonDevice.StateMuteOn:
                    if not self.audio.setMute(True): self.currentState = MuteButtonDevice.StateError
                    self.isMutedLastRead = 0
                self._externalCallState = None

            if self._externalCallVolume is not None:
                dbgPrint(f'App.run. External change. Volume -> [{self._externalCallVolume}]')
                self.currentVolume = self._externalCallVolume
                if self.currentVolume < 0: self.currentVolume = 0.0
                if self.currentVolume > 100: self.currentVolume = 100.0
                self._externalCallVolume = None

            if (currTime - self.currentVolumeLastRead) > 1.5: 
                self.currentVolumeLastRead = currTime
                dbgPrint(f'App.run. Getting current volume....')
                currVol = self.audio.getVolume()
                if currVol is None: self.currentState = MuteButtonDevice.StateError
                else: self.currentVolume = currVol

            if (currTime - self.isMutedLastRead) > 1.0:
                self.isMutedLastRead = currTime
                dbgPrint(f'App.run. Getting current mute state....')
                currMute = self.audio.getMute()
                if currMute is None: self.currentState = MuteButtonDevice.StateError
                else: self.currentState = MuteButtonDevice.StateMuteOn if currMute else MuteButtonDevice.StateMuteOff

            if self.currentState != self.lastState or self.lastVolume != self.currentVolume:
                self.lastState = self.currentState
                self.lastVolume = self.currentVolume
                dbgPrint(f'App.run. ** Data changed [State: {self.currentState}; Volume: {self.currentVolume}]. setting....')
                self.dev.setState(self.currentState, int(self.currentVolume))
                if self.event is not None: self.event(self.currentState, int(self.currentVolume))


    def join(self) -> None:
        self._keepRunning = False
        if self.dev is not None: self.dev.join()


    def externalCall(self, state: int, volume: int):
        dbgPrint(f'App.external call. State: [{state}]; Volumne: [{volume}]')
        if state is not None: self._externalCallState = state
        if volume is not None: self._externalCallVolume = volume


    def deviceOnline(self, dev: MuteButtonDevice) -> bool:
        if dev is None: return False
        return dev.isAlive()


    def setDevice(self, isMuted: bool, currentVol: int) -> MuteButtonDevice:
        dev = MuteButtonDevice.findMuteButtonDevice()

        if dev is None: 
            dbgPrint(f'App.setDevice. Mute button device not found :-(')
            time.sleep(1.0)
            return None

        dbgPrint(f'App.setDevice. Mute button device found in port [{dev.serialPort.portstr}]')

        dev.configureColor([0x00001f00, 0x0000001f, 0x001f0000])
        dev.setState(isMuted, int(currentVol))
        dev.start()

        return dev



#______________________________________________________________________________
#______________________________________________________________________________
