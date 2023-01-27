#!/usr/bin/env bash

#git clone git@github.com:stratosphere/stratosphere-sopremo.git && cd stratosphere-sopremo/meteor/meteor-example/resources/tpch.json/

# join into a single file
echo '[' > o
for i in *.json; do
 sed -e '1d' -e '$d' $i >> o
 echo ',' >> o
done
sed -i '' -e '$d' o
echo ']' >> o

# make primary/foreign keys negative and unique
updateIds() {
  sed -i '' -e "s/\($1\": \)/\1$2/g" o
}
updateIds "custkey" "-10"
updateIds "nationkey" "-20"
updateIds "orderkey" "-30"
updateIds "partkey" "-40"
updateIds "regionkey" "-50"
updateIds "suppkey" "-60"

# rename primary key fields to "entityId" & add "entityDt"
updateFields() {
  addEntityDt $1 $2
  renamePrimaryKey $1
}
addEntityDt() {
  sed -i '' -e "/\"$1/ a\\
  \    \"entityDt\": \"$2\",
  " o
}
renamePrimaryKey() {
  sed -i '' -e "s/\"$1/\"entityId/g" o
}
updateFields "c_custkey" "customerDt"
updateFields "n_nationkey" "nationDt"
updateFields "o_orderkey" "orderDt"
updateFields "p_partkey" "partDt"
updateFields "r_regionkey" "regionDt"
updateFields "s_suppkey" "supplierDt"

addEntityDt "ps_suppkey" "partSupplierDt"
addEntityDt "l_suppkey" "lineItemDt"

# generate output
mv o tpch.json
