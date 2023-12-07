.. |bandcamp image| image:: https://github.com/maximus-lee-678/MadeInBandcamp/blob/main/MadeInBandcamp/assets/madeinbandcamp.jpg
  :width: 24px
  :alt: MadeInBandcamp Logo

|bandcamp image| MadeInBandcamp
=================

| Windows utility to download 128kbps tracks from Bandcamp.
| Automatically performs ID3 tagging on downloaded songs.
|
| Supported download types:

* Track
* Album
* Artist

👾 Usage
---------

1. Specify any combination of tracks, albums or artists.
2. Enter "\*\*\*" to begin downloads.
3. Avoid tampering the "temp" or "songs" folder during operation.

* 🎵 **Track**

.. code-block:: console

  https://[?].bandcamp.com/track/[?]

* 🎶 **Album**

.. code-block:: console

  https://[?].bandcamp.com/album/[?]

* 🎼 **Artist**

.. code-block:: console

  https://[?].bandcamp.com
  https://[?].bandcamp.com/music

🚓 Roadmap
-----------

* Currently does not properly support songs with UTF-8 encoding, retrieves the song but does not load song name or metadata properly.
* Update to new CLI display method.

🖥️ VS2022 Setup
---------------

This program was developed in VS2022 using libcurl for Windows.
libcurl was installed using `vcpkg <https://github.com/microsoft/vcpkg>`_.

1. Get `latest <https://github.com/microsoft/vcpkg/releases>`_ vcpkg zip file and extract it.
2. Open **Developer Command Prompt for VS** and cd to wherever vckpg has been extracted to.
3. Run:

.. code-block:: console

  bootstrap-vcpkg.bat

4. Run:

.. code-block:: console

  vcpkg.exe integrate install

5. Run:

.. code-block:: console

  vcpkg.exe install curl

OR

.. code-block:: console

  vcpkg.exe install curl --triplet=x64

6. After installation, in the project: ensure solution configuration (next to debug buttons) is targeting right triplet (x86 or x64).

ℹ️ Disclaimer
---------------

`Isn't this unfair to the artist? <https://get.bandcamp.help/hc/en-us/articles/360007902173-I-heard-you-can-steal-music-on-Bandcamp-What-are-you-doing-about-this>`_

If you enjoy an artist's work, please support them!
