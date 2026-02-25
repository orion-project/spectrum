# Add Graph From Formula

```
► Add ► From Formula...
```

This data source allows for the generation of graph data from a custom formula written in [Lua](http://www.lua.org) code. The code just has to initialize two global variables `X` and `Y` as [arrays](./lua_primer.md#lua_array) of the same size.

## Presets

Use the **Star** button on the toolbar to store current code as a preset to reuse it later for new formulas. The arrow at the right of the buttons pops up a menu showing all saved presets. Click a preset name to put its code into the editor. Note that presets can also be *inserted* into the code instead of replacing the whole code. Use the small menu button following the preset name and click the **Insert Into Code** command. This allows you to store not only full-fledged formulas but also to keep a collection of useful code snippets or reusable functions.

## Example

Here is an example of a formula for making a noisy Gaussian profile:

```lua
y_max = 1000
num_points = 101
center = 0
scan_range = 20
width = scan_range / 10.0
step_x = scan_range / (num_points - 1)
x = center - scan_range / 2
noise_level = 5 -- percent of max
background = y_max * 0.1

X = {}
Y = {}

for i = 1, num_points do
  profile = y_max * exp(-((center-x)^2) / (2 * width^2))
  noise = math.random(0, noise_level)/100*y_max
  X[i] = x
  Y[i] = profile + noise + background
  x = x + step_x
end
```

## See Also

- [Lua Script Primer](./lua_primer.md)