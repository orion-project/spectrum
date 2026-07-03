# Reopen Graph

```
► Graph ► Reopen
```

Read graph points from a different data source (e.g., data file), apply the same transformations, and replace the current graph.

Depending on the graph's data source, the command can behave differently. For example, for file-based data sources, it shows a dialog for choosing of a new file. For [formula](./add_formula.md) data sources, it opens a formula editor dialog and allows for changing the formula code.

Note that when several graphs are selected, the new source (file name, formula code) will be applied on all of them, so they can become indistinguishable because of reading data from the same source. In general, this is undesirable, so before invoking the command pay attention on which graphs are selected. But this behaviour is very useful in case of [CSV File](./add_csv.md) data sources where several graphs are produced from the same file but different columns. Then you can quickly replot the whole set of graphs from another file.

## See Also

- [Refresh Graph](./refresh.md)
