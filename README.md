# ReSequancer

## **Downgrades v10 aseq headers to v7**

```Drag & Drop the v10 aseq into R5ReSequancer```

# Splinter

### **- Transfers bones from one arig/rmdl to an other arig/rmdl**

```
Usage : R5Splinter.exe <source> <target> <source version> <options>

==============================================
Usage       : replace
Example     : R5Splinter.exe ./a.rmdl ./b.rmdl 10 replace
Description : replaces targets bones with source bones
==============================================
Usage       : move -> <source bone name> <pos vector>
Example     : Example : R5Splinter.exe ./a.rmdl ./b.rmdl 10 move ja_c_propGun 0 0 20
Description : moves the specified bone along with it's children
==============================================
Usage       : movetobone -> <source bone name> <target bone name>
Example     : Example : R5Splinter.exe ./a.rmdl ./b.rmdl 10 movetobone ja_c_propGun jx_c_origin
Description : moves the source bone along with it's children to target bones location
==============================================
```

# VGMover

### **- Moves VG Verts**

```
Usage : VGMover.exe <source> <pos x y z> <rot x y z> <scale>

==============================================
Usage       : replace
Example     : VGMover.exe ./a.vg 0 10.0 0 0 180 180 0.8
Description : Moves the vg vertex positions
==============================================
```
