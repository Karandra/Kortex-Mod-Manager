# Frequently asked questions

## Is it a fork of the Mod Organizer?
Not in any way, it's completely independent project. Kortex does share the general idea with Mod Organizer and has some overlapping features but that doesn't make it a rip-off or anything like that.

## What about Vortex, the new mod manager by NexusMods?
Indeed Kortex is named after Vortex but it wasn't my idea. I needed a name starting at "K" because of the program long history. I was about to release v1.0 and then Vortex has been announced. I had no good variants for a name so I asked some people and one of them suggested "Kortex". Because of lack of better options I've chosen that name.

Once again, I started Kortex development in July 2017, about the same time as Vortex I think. And before that I've had two generations of mod managers (**KMM Gen.: 0** and **KMM Gen.: 1** as I call them). Their development started at December 2011 and February 2016 respectively.

## What are the differences between Kortex and other mod managers?
Please look at the [comparison table](?page=comparison).

## I've heard that Kortex VFS is slow. Is that true?
No, it's not true. Well, actually it depends. In comparison to pure NTFS (no VFS) it's slow. Not so much in comparison to other virtual file system implementations. That said MO1 and MO2 solutions are faster than KxVFS and Vortex not-so-much-VFS is faster than MO. KxVFS in Kortex v2.x is faster than in v1.x.

The VFS is certainly fast enough to play the game.

## Does Kortex only support Bethesda games?
Yes at the moment. Other games can be added in the future. In theory Kortex should be able to support almost any game but there is no simple way to add completely new game. There is **Sacred 2** as an example of non-Bethesda game.

You can try to add a new game by creating game <span tooltip="It's an XML file that describes how Kortex should interact with the game">definition file</span> in **Data\GameDefinitions** folder. The format of game definition files can change without any notice when new version is released and there are no documentation about the format details. **I will not provide any support for user created game definitions**.

## Can I manage ENB/DInput/Creation Kit/etc with Kortex?
Yes. Kortex virtualizes game's root folder so mods that need to be installed there are supported.
