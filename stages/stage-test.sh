rm -rf ./stage-tmp
mkdir stage-tmp
mkdir stage-tmp/results
mkdir stage-tmp/results/results-aa01-in-bom.l.dns.icann.org
./ithitools -o ./stage-tmp/results/results-aa01-in-bom.l.dns.icann.org/20210501-000423_300.cbor.xz-results.csv -Y data/tiny-capture.cbor
./ithitools -o ./stage-tmp/results/results-aa01-in-bom.l.dns.icann.org/20210502-000423_300.cbor.xz-results.csv -c data/tiny-capture.pcap
python3 stats/load_l_root_data.py stage-tmp/results stage-tmp/sum3_test.sum3
if diff -q stage-tmp/sum3_test.sum3 data/sum3_test_ref.sum3; then
    echo "Sum3 test passes."
else
    echo "Sum3 test does not produce the expected result."
    exit 1
fi


