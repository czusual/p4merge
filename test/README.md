## For example
``` 
p4merge basic.p4 basic_tunnel.p4
```

For now, this command generate a rough merged p4 file :merge_result.p4

``` 
p4merge merge_result.p4 basic_tunnel.p4 --second-round
```
Add '--second-round' in the end of cmd input, where "merge_result.p4" is the merged p4 file of the first round merging. So that we can merge three files in two rounds.



```
p4merge --help
```

See the help for basic options, provided by p4c 
