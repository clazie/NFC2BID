=========================
        NFC2BID
=========================

This project is based on a project from Rainer Kembügler ("onki") from  02. Dezember 2016 
(seen here: `https://onki.de/uebermich/bauberichte/289-bid2nfc-einbau-ins-ladegeraet <https://onki.de/uebermich/bauberichte/289-bid2nfc-einbau-ins-ladegeraet>`_ )

Information can also found here: `https://www.rc-network.de/threads/bid2nfc-als-nachr%C3%BCstl%C3%B6sung.587281/ <https://www.rc-network.de/threads/bid2nfc-als-nachr%C3%BCstl%C3%B6sung.587281/>`_

I collected all informations necessary to build it by my own and refactored the firmware for 
using it with PlatformIO.



..  Copyright 2014-present PlatformIO <contact@platformio.org>
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

How to build PlatformIO based project
=====================================

1. `Install PlatformIO Core <http://docs.platformio.org/page/core.html>`_
2. Download `development platform with examples <https://github.com/platformio/platform-atmelavr/archive/develop.zip>`_
3. Extract ZIP archive
4. Run these commands:

.. code-block:: bash

    # Change directory to example
    > cd platform-atmelavr/examples/arduino-blink

    # Build project
    > platformio run

    # Upload firmware
    > platformio run --target upload

    # Build specific environment
    > platformio run -e uno

    # Upload firmware for the specific environment
    > platformio run -e uno --target upload

    # Clean build files
    > platformio run --target clean
