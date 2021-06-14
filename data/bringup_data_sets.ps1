$COMMIT=$(cat bringup_data_sets_sha.txt)

git clone https://git.stabletec.com/foe/bringup-data-sets.git
cd bringup-data-sets
git checkout $COMMIT