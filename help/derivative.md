# First Derivative

```
► Modify ► First Derivative...
```
The function calculates the first derivative of the selected graph.

## Parameters

![Derivative](./images/modify_derivative.png)

Mode defines a formula used for computing of Y values of the result graph.

### Simple

<span style="font-family:Times New Roman;font-size:14pt"><i>
dY<sub>i</sub> = (Y<sub>i</sub> - Y<sub>i-1</sub>) / (X<sub>i</sub> - X<sub>i-1</sub>)
</i></span>

The result graph has one point less than the original one.

### Refined

Each point is calculated by averaging of the left and right intervals.

<span style="font-family:Times New Roman;font-size:14pt"><i>
dY<sub>left,i</sub> = (Y<sub>i</sub> - Y<sub>i-1</sub>) / (X<sub>i</sub> - X<sub>i-1</sub>)
</i></span>

<span style="font-family:Times New Roman;font-size:14pt"><i>
dY<sub>right,i</sub> = (Y<sub>i+1</sub> - Y<sub>i</sub>) / (X<sub>i+1</sub> - X<sub>i</sub>)
</i></span>

<span style="font-family:Times New Roman;font-size:14pt"><i>
dY<sub>i</sub> = (dY<sub>left,i</sub> + dY<sub>right,i</sub>) / 2
</i></span>

The result graph has the same number of points than the original one. The first and the last points are calculated using the Simple algorithm.

### Simple with Tau

A given fixed value used for <span style="font-family:Times New Roman;font-size:14pt"><i>dX</i></span>.

<span style="font-family:Times New Roman;font-size:14pt"><i>
dY<sub>i</sub> = (Y<sub>i</sub> - Y<sub>i-1</sub>) / τ
</i></span>

The result graph has one point less than the original one.

### Refined with Tau

Each point is calculated by averaging of the left and right intervals. A given fixed value used for <span style="font-family:Times New Roman;font-size:14pt"><i>dX</i></span>.

<span style="font-family:Times New Roman;font-size:14pt"><i>
dY<sub>left,i</sub> = (Y<sub>i</sub> - Y<sub>i-1</sub>) / τ
</i></span>

<span style="font-family:Times New Roman;font-size:14pt"><i>
dY<sub>right,i</sub> = (Y<sub>i+1</sub> - Y<sub>i</sub>) / τ
</i></span>

<span style="font-family:Times New Roman;font-size:14pt"><i>
dY<sub>i</sub> = (dY<sub>left,i</sub> + dY<sub>right,i</sub>) / 2
</i></span>

The result graph has the same number of points than the original one. The first and the last points are calculated using the Simple with Tau algorithm.
