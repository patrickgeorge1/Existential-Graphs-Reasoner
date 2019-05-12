# Existential Graphs Reasoner


### Nume: Mihai Pirvulet & Vitoga George Patrick
### Grupa: 312CA
### Start date: 05:05:2019
### End date: 09:05:2019

  WARNING : Fisierul .h a fost modificat. Don't overwrite it pls :)

## Encountered Problems :
* had to use recursion to solve some tasks
* deiterate() and erase() don't take in account the level of the deleted node, so task 7 fails, the position where the graph string needs change should be calculated with split_level() or some similar mechanic that iterates through the subgraphs and atoms.

## Solutions :
* used additional functions "helpers" to have access in every call to our values

 	 `possible_double_cuts_helper(*this, path, res);`
* used `cutPosition` to iterate trough the graph, split levels, split graph and `found` will keep the position in the string where deiterate will occur

```
  AEGraph cutPosition = graphCopy, parent("()");
  
  	// preparation for padding and splitting the level
  
  auto elements = split_level(representation);
  
  	//  found time
  
  for (unsigned int i = 0; i < elements.size() && i < (unsigned int) where[where.size() - 1]; ++i) {
   
    found+=elements[i].length();
    
    found+=2;  // add separators -> ", "
    
  }
  ```
  ## Division of the tasks :
  
  ### Mihai Pirvulet :
  * possible_double_cuts_helper()
  * possible_double_cuts()
  * double_cut()
  * possible_deiterations()

  
  
  ### Vitoga George Patrick :
  * possible_erasures_helper()
  * possible_erasures()
  * erase()
  * coding style
  * README.md
