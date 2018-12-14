
    Vector for Psion 3a/3c - Version P1.20.b5

    Copyright (c) 1997, 1998, Nick Matthews, KizzyCo

This beta program is provided for testing purposes only. If you do not
already have the current release version (P1.11) you should obtain it
from the KizzyCo web site http://www.kizzyco.com/vector.htm - file
vecp111.zip, which contains the Vector Manual and example drawings as
well as the current release program.

You are encouraged to comment on this program (that is the point of it)
by email to nick@kizzyco.com. All comments are welcome, not just bug
reports. In particular, please let me know of any success or failure in
using the Export/Import options - be sure to give the full details of
the package used to view and/or create the files.

    Nick Matthews
    KizzyCo.
    74 High Street
    Ardingly
    West Sussex RH17 6TD
    England

    Tel: +44 (0)1444 892560
    Email: nick@kizzyco.com

    Web: http://www.kizzyco.com/

Beta Notes
~~~~~~~~~~
Access the DXF Export function from the "File, Save as" menu option
and the DXF Import function from the "File, Open file" option. Note
that you need to set a suitable scale in the "Preference, Units & scale"
dialog (see below) before importing a DXF file.

DXF files are always exported with the Y direction set to 'Positive = Up'
although the default setting is currently 'Positive = Down' (It is
intended to change the default settings before the final release)

The "Preferences, Units" dialog has been expanded and is now called
"Units & scale" dialog. A new "Preference, Origin" dialog has been
added to position absolute units. All dialog boxes that use dimensions
should say (in the title) whether they are expecting values in Paper
or Scale units.

NOTE. Vector version P1.20.b4 and later will read drawing files created
with previous versions, BUT saved files cannot be read by previous
versions of the program. Please ensure that any drawing files you are
using are backed up first, so you can go back to using the P1.11
release version if necessary.

File List
~~~~~~~~~
  README_.TXT  This file
  VECTOR.APP   The Vector Beta program
  VECDXL.VDL   DXL Import/Export dynamic library

Installation
~~~~~~~~~~~~
  Place VECTOR.APP in the \APP\ folder (directory) on any drive
  Place VECDXF.VDL in the \APP\VECTOR\ folder on the same drive

  Note, the *.VDL file MUST be in the \APP\VECTOR\ folder on the same
  drive as the VECTOR.APP file. Also, both files must be the same
  version (from the same distribution).

Distribution
~~~~~~~~~~~~
  Do NOT distribute this program.
  Only the current release version may be distributed.
  See the current release version for registration details.

History
~~~~~~~
P1.20.b5  vecp12b5.zip  11th Dec 1998

  1. Dimension menu option and sub menu added. (not yet functioning)
  2. DXF Export, Bugs related to movable origin and Y direction corrected
  3. DXF Export, Ommisions in file causing problems with AutoCAD corrected

P1.20.b4  vecp12b4.zip  29th Nov 1998

  1. When config file is saved only updated section is changed
  2. Movement key settings and cursor style are now saved with config file
  3. Units dialog changed to Units and scale and scale settings added
  4. Various dialogs now shows Scale or Paper units and values.
  5. Preferences sub menu reorganised
  6. 'Preferences, Origin' option and dialog box added.
  7. Set direction of y scale added to "Origin" dialog box
  8. Default new file units is now mm (was cm)
  9. 'Show page' dialog shown after changing Page, Units & scale or Origin
 10. Symbol selection now displays correctly when in Quick Draw mode.
 11. Info window displays Scale in step position during Viewing mode
 12. DXF import, rotating and scaling of blocks corrected
 13. DXF import, unknown sections now skipped instead of causing error
 14. DXF import, layers are now correctly switched on or off
 15. DXF export, Snap and Grid details added to header
 16. RegNet registration details updated in help pages
 17. Serial printing now allows for higher 3mx speed.

P1.20.b3  vecp12b3.zip  19th Oct 1998

  1. Implemented DXF Import in VECDXF.VDL dynamic library file
  2. Corrected bug when displaying long text strings.
  3. 'Zoom to, Extent" now ignores switched off layers
  4. Saving busy message put up during file save
  5. DXF export cancel dialog gives count of saved elements

P1.20.b2  vecp12b2.zip 15th Sep 1998

  1. Fixed break on 3 pt arc where a remaining arc was over 180 deg.
  2. Added DXF Export to "Save as" dialog, implemented in VECDXF.VDL
  3. Added DXF Import to "Open" dialog but not implemented yet.

P1.11   vecp111.zip

  Released 13th May 1998

Disclaimer
~~~~~~~~~~
THIS PROGRAM IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, EXPRESSED OR
IMPLIED, INCLUDING BUT NOT LIMITED TO FITNESS FOR A PARTICULAR PURPOSE.
UNDER NO CIRCUMSTANCES WILL THE AUTHOR (Nick Matthews) BE HELD
RESPONSIBLE FOR LOSS OF DATA, DAMAGE TO SOFTWARE/HARDWARE, OR ANY OTHER
TYPE OF LOSS OR DAMAGE AS A DIRECT OR INDIRECT RESULT OF USING THIS
SOFTWARE.

11th December 1998
