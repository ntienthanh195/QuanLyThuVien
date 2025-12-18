Migration guidelines — QTableWidget -> QTableView + QAbstractTableModel

Goal
- Reduce UI allocations and improve responsiveness by using model/view backed tables.

Pattern
1. File-local model
   - Implement a small `QAbstractTableModel` in the `.cpp` (anonymous namespace or file-local class).
   - Keep the model implementation simple: hold a `std::vector<T>` or `PTR` array snapshot and expose `rowCount`, `columnCount`, `data`, and `headerData`.
   - Add a `refresh()` method that calls `beginResetModel()`/`endResetModel()` after updating the backing vector.

2. Backing data
   - When reading from legacy linked lists or global arrays, snapshot pointers into a `std::vector` before doing heavy work.
   - Offload expensive aggregations (counts, statuses) to a worker thread and apply results via `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`.

3. Styling and delegates
   - Use a `QStyledItemDelegate` (file-local or shared) for custom painting (badges, icons) to avoid per-cell widgets.
   - Attach the delegate with `table->setItemDelegateForColumn(colIndex, delegate)`.

4. Row/column sizing
   - Avoid loops calling `setRowHeight()` per row. Instead call `verticalHeader()->setDefaultSectionSize(height)` once.
   - Avoid repeated `resizeColumnsToContents()` calls; prefer sensible defaults or compute widths once after model population.

5. Pagination
   - Implement simple pagination inside the model (page size + page index). Expose `setPageSize`, `setPage`, `totalPages`, `totalCount`.
   - Persist page-size choice via `QSettings` for better user experience.

6. Minimize header changes
   - Prefer adding models in `.cpp` files to avoid changing public headers which trigger moc/extra recompiles.

7. Selection mapping
   - Map view-selected row to model item using `model->getBookAt(viewRow)` and refresh from canonical backend if needed (e.g., `TimDauSachTheoISBN(...)`).

Examples and quick checklist
- Replace `QTableWidget` with `QTableView` + `TopModel`.
- Backing vector: `std::vector<TopDTO> danhSach; model->setData(&danhSach); model->refresh()`.
- Use `beginResetModel()/endResetModel()` for full refresh; for incremental updates implement fine-grained `beginInsertRows`/`endInsertRows` only if needed.

Performance notes
- Snapshot pointers before iterating; avoid traversing linked lists on the GUI thread for each cell.
- Batch compute expensive aggregates (counts/status) off-thread and update caches used by `data()`.
- Avoid creating/destroying delegates/widgets per cell.

Follow-up
- Consider adding perf regression tests that measure `model->refresh()` time on representative datasets (see `tools/` for potential scripts).

