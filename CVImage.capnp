@0xba8893f6fe8d8429;

# Resembles MAT object from CV

struct CVImage {
    rows @0 :UInt32;    # Rows in MAT Image; MAT.rows
    cols @1 :UInt32;    # Cols in MAT Image; MAT.cols
    type @2 :UInt32;    # Type of MAT Image; MAT.type()
    data @3 :Text;      # Data (actual Image) of MAT Image; MAT.data
}
