- infos = Information about keytometa plugin is in keys below
- infos/author = Felix Berlakovich <elektra@berlakovich.net>
- infos/licence = BSD
- infos/needs =
- infos/provides = conversion
- infos/placements = presetstorage postgetstorage
- infos/description = conversion of keys to meta keys and vice versa

## INTRODUCTION ##

This plugin converts keys into metakeys of other keys. 
The keys to be converted are tagged with special metadata. 
Converting keys into metakeys basically raises two questions:
- which keys are to be converted
- which key to append the resulting metakeys to

The keys to be converted are identified by metakeys below "convert" (e.g. "convert/append"). 
The keys receiving the resulting meta data are identified by append strategies. 
The plugin currently supports the following metakeys for controlling the conversion:

- convert/metaname: specifies the name of the resulting metakey. For example taggng the key user/config/key1 with "convert/metaname = comment" means that the key will be converted to a metakey with the name "comment".
- convert/append: specifies the append strategy (see below)
- convert/append/samelevel: specifies that the key should only be written to the metadata of a key with the same hiearchy level (see below). 

The keys converted to metadata are restored as soon as the keyset is written back. 
However, the plugin is stateful. This means that a keyset must be read and keys must be 
converted by the plugin in order to undo this conversion in the set direction. 
Modifications to the metadata which resulted from converted keys are propagated back 
to the corresponding key (see merging for more details).

The keys are ordered by the "order" metadata. If two keys are equal according to the order metadata, 
they are ordered by name instead.


 
## APPEND STRATEGIES ##

The append strategy specifies which key will receive the resulting metadata. 
Currently the plugin supports the following strategies:

### PARENT STRATEGY ###

The metadata is added to the first existing parent of the converted key. 
This does not neccessarily have to be the parent of the keyset. If no such key is found, 
the first key in a sorted keyset will receive the metadata (this is usually the parent key of the keyset). 
For example consider the following keyset:

				user/config/key1
				user/config/key1/child1
				user/config/key2he t
				user/config/key2/deeper/child2
				user/config/key3

If child1, child2 and child3 were tagged with "convert/append = parent", key1 would receive 
the metadata from child1 and child3 and key2 would receive the metadata from child2. 

### NEXT STRATEGY ###

The metadata is added to the key following the converted key in a sorted keyset. 
If no such key is found (for example because the key to be converted is the last one), 
the strategy is reverted to parent. For example consider the following keyset:

				user/config/deeper/key1
				user/config/key2
				user/config/key3
				user/config/key4

If key1 and key3 were tagged with "convert/append = next", key2 would receive the metadata 
resulting from key1 and key4 would receive the metadata resulting from key3.

### PREVIOUS STRATEGY ###

The metadata is added to the key preceeding the converted key in a sorted keyset. 
If no such key is found (for example because the key to be converted is the first one), 
the strategy is reverted to parent. For example consider the following keyset:

				user/config/key1
				user/config/deeper/key2
				user/config/key3
				user/config/key4

If key2 and key4 were tagged with "convert/append = previous", key1 would receive the metadata 
resulting from key2 and key3 would receive the metadata resulting from key4.



## MERGING ##

The metadata resulting from a converted key is is never appended to another key which is going to 
be converted. This prevents that the data of converted keys is invisible after the conversion. 
Instead the metadata resulting from different converted keys with the same append strategy is 
merged together (separated by a newline). Keys with different append strategies are skipped, 
until either a key with the same strategy is found (which is simply merged as decribed above) 
or the target key is found. The keys are always processed in the order of an ordered keyset. 
For example consider the following keyset:

				user/conifg/key0
				user/config/key1 = value1
				user/config/key2 = value2
				user/config/key3 = value3
				user/config/key4 = value4
				user/config/key5

If key1 and key2 were tagged with "convert/append = next" and key3 and key4 were tagged with "convert/append = previous" the following would happen:
- the resulting metadata of key0 would contain "value3\nvalue4" (the values of key3 and key4 are merged together and key1 and key2 are skipped, as they have differnt append strategy)
- the resulting metadata of key5 would contain "value1\nvalue2" (the values of key1 and key2 are merged together and key3 and key4 are skipped, as they have different append strategy)



### SAMELEVEL APPENDING ###

The option "convert/append/samelevel" can be used to force that the metadata is only appended to a key on the same hierarchy level. If no such key is found, the strategy is reverted to parent. Note, that the value of the samelevel key does not matter. Only its existence is relevant. For example consider the following keyset:

				user/config/key0
				user/config/key1/child1
				user/config/key1
				user/config/key2/child2
				user/config/key2
				user/config/key3
				user/config/key4

If child1, child2 and key3 were each tagged with "convert/append = next" and child2 and key3 were tagged with "convert/append/samelevel", key1 would receive the metadata resulting from child1, key0 would receive the metadata resulting from child2 (strategy reverted to parent) and key4 would receive the metadata resulting from key3.
