// SPDX-License-Identifier: GPL-2.0
// Cells of the threads in columns, every kind of the threads (P, E, D) in its own columns (never mixed in a column),
// the kinds in the given order, column by column; as few rows as possible so that the columns fit maxColumns
.pragma library

// groups: arrays of the CPU numbers in the order of the kinds; result: { rows, columns, positions: { cpu: [row, column] } }
function place(groups, maxColumns) {
    const nonEmpty = groups.filter(group => group.length > 0)
    const total    = nonEmpty.reduce((sum, group) => sum + group.length, 0)
    const limit    = Math.max(maxColumns, nonEmpty.length)
    let rows = 1

    while (rows < total && nonEmpty.reduce((sum, group) => sum + Math.ceil(group.length / rows), 0) > limit)
        ++rows

    // One column per kind if it makes the grid at most 25 % higher: no column with a few threads left over
    const longest = nonEmpty.reduce((max, group) => Math.max(max, group.length), 0)

    if (longest > rows && longest <= rows * 1.25 && nonEmpty.length <= limit)
        rows = longest

    const positions = {}
    let column = 0

    for (const group of nonEmpty) {
        for (let i = 0; i < group.length; ++i)
            positions[group[i]] = [i % rows, column + Math.floor(i / rows)]

        column += Math.ceil(group.length / rows)
    }

    return { rows: Math.max(rows, 1), columns: column, positions: positions }
}
