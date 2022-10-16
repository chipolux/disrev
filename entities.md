.entities files seem to define where all the objects and effects are in a map.

the z coordinate is vertical

corvo is roughly 2 units tall

seem to be able to add this to the edit block of any idEntity and idDynamicEntity
to change the scale of the model

```
renderModelInfo = {
    scale = {
        x = 1.0;
        y = 1.0;
        z = 1.0;
    }
}
```
