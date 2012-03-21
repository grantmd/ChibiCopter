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