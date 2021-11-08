echo 'Executing central node script'
echo 'Building central node'
make && \
chmod +x execute_ground && \
echo 'Executing central node program' && \
./execute_ground
echo 'Central node program complete, cleaning up'
make clean
echo 'Done!'