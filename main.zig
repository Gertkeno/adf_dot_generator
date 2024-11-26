const std = @import("std");

const String = []const u8;

// -OReleaseSmall
// ==31098==   total heap usage: 17 allocs, 17 frees, 54,664 bytes allocated
// -OReleaseSafe
// ==31401==   total heap usage: 17 allocs, 17 frees, 60,480 bytes allocated

const Page = struct {
    name: String,
    links: std.SinglyLinkedList(String),
    breaks: std.SinglyLinkedList(String),
};

fn writeLinks(writer: anytype, links: std.SinglyLinkedList(String), count: usize, color: bool) !void {
    const cstr = if (color) " [color=blue]" else "";
    if (count == 0) {
        try writer.writeAll(";\n");
    } else if (count == 1) {
        try writer.print(" -> {s}{s};\n", .{ links.first.?.data, cstr });
    } else {
        var linkhead = links.first;
        try writer.writeAll(" -> { ");
        while (linkhead) |node| {
            try writer.print("{s} ", .{node.data});
            linkhead = node.next;
        }

        try writer.print("}}{s};\n", .{cstr});
    }
}

fn freeStringList(allocator: std.mem.Allocator, list: std.SinglyLinkedList(String)) void {
    var linkhead = list.first;
    while (linkhead) |node| {
        linkhead = node.next;
        allocator.free(node.data);
        allocator.destroy(node);
    }
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){
        .backing_allocator = std.heap.c_allocator,
    };
    defer _ = gpa.deinit();
    var logging_allocator = std.heap.loggingAllocator(gpa.allocator());
    const allocator = logging_allocator.allocator();

    var args = try std.process.argsWithAllocator(allocator);
    _ = args.skip();
    const cwd = std.fs.cwd();

    const stdout = std.io.getStdOut().writer();
    const stderr = std.io.getStdErr().writer();
    while (args.next()) |arg| {
        if (cwd.openFileZ(arg, .{})) |file| {
            const size = (try file.metadata()).size();
            const file_buffer: []u8 = try allocator.alloc(u8, size);
            defer allocator.free(file_buffer);
            _ = try file.read(file_buffer);

            var pages = std.ArrayList(Page).init(allocator);
            defer {
                for (pages.items) |page| {
                    allocator.free(page.name);
                    freeStringList(allocator, page.links);
                    freeStringList(allocator, page.breaks);
                }
                pages.deinit();
            }

            var page: *Page = undefined;

            var line_slicer = std.mem.tokenizeScalar(u8, file_buffer, '\n');
            while (line_slicer.next()) |line| {
                const trimmed = std.mem.trim(u8, line, &std.ascii.whitespace);
                if (trimmed[0] == '#') {
                    const page_name = std.mem.trim(u8, trimmed, "# ");
                    //try stdout.print("Found page: {s}\n", .{page_name});
                    page = try pages.addOne();
                    page.name = try std.ascii.allocLowerString(allocator, page_name);
                    page.links = .{};
                    page.breaks = .{};
                } else if (trimmed[0] == '~') {
                    if (std.mem.indexOfScalar(u8, trimmed, ':')) |dot| {
                        const func = std.mem.trim(u8, trimmed[1..dot], &std.ascii.whitespace);
                        const val = std.mem.trim(u8, trimmed[dot + 1 ..], &std.ascii.whitespace);

                        //try stdout.print("Found fn {s}: {s}\n", .{ func, val });
                        if (std.ascii.eqlIgnoreCase(func, "tag") or
                            std.ascii.eqlIgnoreCase(func, "link"))
                        {
                            const node = try allocator.create(std.SinglyLinkedList(String).Node);
                            node.* = .{
                                .data = try std.ascii.allocLowerString(allocator, val),
                            };
                            page.links.prepend(node);
                        } else if (std.ascii.eqlIgnoreCase(func, "break")) {
                            const node = try allocator.create(std.SinglyLinkedList(String).Node);
                            node.* = .{
                                .data = try std.ascii.allocLowerString(allocator, val),
                            };
                            page.breaks.prepend(node);
                        }
                    } else {
                        try stderr.print("Found function missing end, ~{s}\n", .{trimmed[1..]});
                    }
                }
            }

            try stdout.writeAll("digraph chapter_view{\n");
            for (pages.items) |p| {
                const lcount = p.links.len();
                const bcount = p.breaks.len();
                if (lcount == 0 and bcount == 0) {
                    try stdout.print("    {s};\n", .{p.name});
                } else {
                    if (lcount > 0) {
                        try stdout.print("    {s}", .{p.name});
                        try writeLinks(stdout, p.links, lcount, false);
                    }
                    if (bcount > 0) {
                        try stdout.print("    {s}", .{p.name});
                        try writeLinks(stdout, p.breaks, bcount, true);
                    }
                }
            }

            try stdout.writeAll("}\n");
        } else |err| {
            try stderr.print("Couldn't open file {s}, {}\n", .{ arg, err });
        }
    }
}
