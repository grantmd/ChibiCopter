ChibiCopter
==========

A quadcopter running under chibios (I am running it on an STM32F4 discovery board).

Parts
-----

* STMicroelectronics STM32F4DISCOVERY board: http://www.st.com/internet/evalboard/product/252419.jsp
* 66 Channel LS20031 GPS 5Hz Receiver: http://www.sparkfun.com/products/8975 (although any serial GPS will do)
* Spektrum Remote/Satellite Receiver: http://www.spektrumrc.com/Products/Default.aspx?ProdID=SPM9645

Requirements
------------

* A copy of chibios in ../chibios
* A copy of mavlink in ../mavlink (you will need to build the C message types)

Notes
-----

* A listing of the pins on my STM32FDISCOVERY board, and what I'm mapping them to: https://docs.google.com/spreadsheet/ccc?key=0AkLgoXdu_mAhdE9oV3J5ODFaWEhoT1pUTXRFYXluSUE
* chibios: http://www.chibios.org/
* mavlink: http://qgroundcontrol.org/mavlink/start