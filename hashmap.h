// Copyright 2020 Joshua J Baker. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.

#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/// \author Joshua J Baker
/// An open addressed hash map using robinhood hashing.
struct hashmap;

/// Creates a new hashmap.
/// \param elsize The size of each element in the tree.
/// \param cap The default lower capacity of the hashmap. Setting this to zero will default to 16.
/// \param seed0 Optional seed value passed on to the hash function.
/// \param seed1 Optional seed value passed on to the hash function.
/// \param hash The hash function used for the hashmap.
/// \param compare The function that compares items in the tree. See the qsort stdlib function for an example.
/// \param elfree The function that frees a specific item. This should be NULL unless referenced data is stored in the hash.
/// \param udata A pointer to user-defined data that can be passed to the element comparison and element free functions.
/// \return A pointer to a new hashmap.
struct hashmap *hashmap_new(size_t elsize, size_t cap, 
                            uint64_t seed0, uint64_t seed1,
                            uint64_t (*hash)(const void *item, 
                                             uint64_t seed0, uint64_t seed1),
                            int (*compare)(const void *a, const void *b, 
                                           void *udata),
                            void (*elfree)(void *item),
                            void *udata);

/// Creates a hashmap with an custom allocator.
/// \param malloc A pointer to the allocation function.
/// \param realloc A pointer to the reallocation function.
/// \param free A pointer to the free function.
/// \param elsize The size of each element in the tree.
/// \param cap The default lower capacity of the hashmap. Setting this to zero will default to 16.
/// \param seed0 Optional seed value passed on to the hash function.
/// \param seed1 Optional seed value passed on to the hash function.
/// \param hash The hash function used for the hashmap.
/// \param compare The function that compares items in the tree. See the qsort stdlib function for an example.
/// \param elfree The function that frees a specific item. This should be NULL unless referenced data is stored in the hash.
/// \param udata A pointer to user-defined data that can be passed to the element comparison and element free functions.
/// \return A pointer to a new hashmap.
struct hashmap *hashmap_new_with_allocator(
                            void *(*malloc)(size_t), 
                            void *(*realloc)(void *, size_t), 
                            void (*free)(void*),
                            size_t elsize, size_t cap, 
                            uint64_t seed0, uint64_t seed1,
                            uint64_t (*hash)(const void *item, 
                                             uint64_t seed0, uint64_t seed1),
                            int (*compare)(const void *a, const void *b, 
                                           void *udata),
                            void (*elfree)(void *item),
                            void *udata);

/// Frees the hash map.
/// \param map The hash map to be freed.
void hashmap_free(struct hashmap *map);

/// Quckly clears the whole map.
/// \details Every item is called with the element-freeing function given in hashmap_new,
/// if present, to free any data referenced in the elements of the hashmap.
/// \param map A pointer to the map to be cleared.
/// \param update_cap The new capacity of the hash map (optional).
/// \post  When the update_cap is provided, the map's capacity will be updated to match
/// the currently number of allocated buckets. This is an optimization to ensure
/// that this operation does not perform any allocations.
void hashmap_clear(struct hashmap *map, bool update_cap);

/// Getter for the number of items in the hash map.
/// \param map A pointer to the to get the count from.
/// \return The number of elements in the map.
size_t hashmap_count(struct hashmap *map);

/// Checker if the system is out of memory.
/// \param map A pointer to the map to be checked.
/// \return True if the system is out of memory, false if not.
bool hashmap_oom(struct hashmap *map);

/// Gets an item out of the map with the given key.
/// \param map A pointer to he map to get an element out of.
/// \param key The key of the item to be found.
/// \return The item if found, NULL if the item is not found.
/// \pre Key may not be NULL.
void *hashmap_get(struct hashmap *map, const void *key);

/// Inserts or replaces an item in the hash map.
/// \param map A pointer to the map to insert or replace an item in.
/// \param item The item to be added to the list.
/// \return The item that is replaced, NULL if no item is replaced.
/// \pre Key may not be NULL.
/// \post If the map has reached 75% of its capacity,
/// it resizes the map to double its current size to make room for new entries.
void *hashmap_set(struct hashmap *map, const void *item);

/// Deletes an item from the hash map.
/// \param map A pointer to the map to delete an item from.
/// \param key The key of the item to be deleted.
/// \return The deleted item, NULL if the item is not found.
void *hashmap_delete(struct hashmap *map, void *key);

/// Gets the item in the bucket at a certain position.
/// \param map A pointer to the hashmap.
/// \param position The position of the bucket.
/// The position is reduced to position modulo number of buckets.
/// \return The item, or NULL if no item is present at that bucket.
void *hashmap_probe(struct hashmap *map, uint64_t position);

/// Scanner for the hash map.
/// \param map A pointer to the map to be scanned.
/// \param iter A pointer to the user-provided callback function to be called for each item.
/// \param udata A pointer to user-provided data, which will be passed to the callback function.
/// \return True if the iteration was completed normally, false if it was stopped early.
bool hashmap_scan(struct hashmap *map,
                  bool (*iter)(const void *item, void *udata), void *udata);

/// Iterator for the hash map.
/// \param map A pointer to the hash map to be iterated.
/// \param i A pointer to a size_t cursor that should be initialized to 0 at the beginning of the loop.
/// \param item A void pointer pointer that is populated with the retrieved item. Note that this
/// is NOT a copy of the item stored in the hash map and can be directly modified.
/// \return True if an item was retrieved, false if the end of the iteration has been reached.
/// \note Note that if hashmap_delete() is called on the hashmap being iterated,
/// the buckets are rearranged and the iterator must be reset to 0, otherwise
/// unexpected results may be returned after deletion.
/// \warning This function has not been tested for thread safety.
bool hashmap_iter(struct hashmap *map, size_t *i, void **item);

/// Creates a hash value using SipHash-2-4.
/// \param data The data used for hash generation.
/// \param len The length of the data.
/// \param seed0 A seed for the hashing algorithm (optional).
/// \param seed1 A seed for the hashing algorithm (optional).
/// \return The hash of the data.
uint64_t hashmap_sip(const void *data, size_t len, 
                     uint64_t seed0, uint64_t seed1);

/// Creates a hash value using Murmur3_86_128.
/// \param data The data used for hash generation.
/// \param len The length of the data.
/// \param seed0 A seed for the hashing algorithm (optional).
/// \param seed1 A seed for the hashing algorithm (optional).
/// \return The hash of the data.
uint64_t hashmap_murmur(const void *data, size_t len, 
                        uint64_t seed0, uint64_t seed1);


/// This function allows for configuring a custom allocator for all hashmap library operations.
/// \param malloc A pointer to the allocation function.
/// \param free A pointer to the free function.
/// \deprecated Use `hashmap_new_with_allocator`
void hashmap_set_allocator(void *(*malloc)(size_t), void (*free)(void*));

#endif
