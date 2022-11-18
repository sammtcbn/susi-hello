from ctypes import *
susilib = cdll.LoadLibrary('/lib/libSusiIoT.so')
susilib.SusiIoTGetPFCapabilityString.restype = c_char_p
susilib.SusiIoTInitialize()
capstr = susilib.SusiIoTGetPFCapabilityString()
print (capstr)
susilib.SusiIoTUninitialize()
