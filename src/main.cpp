#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <vector>
#include <list>

using Line = std::pair<sf::Vector2f, sf::Vector2f>;
using Circle = std::pair<sf::Vector2f, double>;

struct PairDesc
{
    double M;
    std::pair<sf::VertexArray, sf::VertexArray> pair;
};

constexpr unsigned WINDOW_HEIGHT_SHIFT = 250;
constexpr unsigned WINDOW_WIDTH_SHIFT = 250;
constexpr double POINTS_SHIFT = 100;
constexpr double MAX_DELTA = 0.5;
constexpr double EPS_SARIEL = 0;

std::ostream& operator << (std::ostream& stream, const Line& line)
{
    return stream << "line{(" << line.first.x << "; " << line.first.y << "), (" << line.second.x << "; " << line.second.y << ")}";
}

inline double RadToDeg(double rad)
{
    return rad*180.0/M_PI;
}

inline double GetDistance2D(const sf::Vector2f& point1, const sf::Vector2f& point2, bool square = false)
{
    double tmp = (point1.x - point2.x)*(point1.x - point2.x) + (point1.y - point2.y)*(point1.y - point2.y);
    return square ? tmp : std::sqrt(tmp);
}

inline double GetDotProduct2D(const sf::Vector2f& point1, const sf::Vector2f& point2)
{
    return point1.x*point2.x + point1.y*point2.y;
}

inline double GetLinePointY(const Line& line, double x)
{
    return ((line.first.y - line.second.y)*x + (line.first.x*line.second.y - line.second.x*line.first.y))/(line.first.x - line.second.x);
}

sf::Vector2f GetLineNormalIntersectPoint(const Line& line, const sf::Vector2f& point)
{
    float x = ((line.first.x - line.second.x)*(line.first.x - line.second.x)*point.x +
                (line.first.y - line.second.y)*(line.first.x - line.second.x)*point.y -
                (line.first.x*line.second.y - line.second.x*line.first.y)*(line.first.y-line.second.y))/
                ((line.first.y - line.second.y)*(line.first.y - line.second.y) + (line.first.x - line.second.x)*(line.first.x - line.second.x));
    float y = GetLinePointY(line, x);
    return {x, y};
}

inline bool IsPointLeftToLine(const Line& line, const sf::Vector2f& point)
{
    return point.y < GetLinePointY(line, point.x);
}

double GetAngle(const Line& line1, const Line& line2)
{
    auto fwd_vec1 = line1.second - line1.first, fwd_vec2 = line2.second - line2.first;
    double sz1 = GetDistance2D(line1.first, line1.second), sz2 = GetDistance2D(line2.first, line2.second);
    return acos(GetDotProduct2D(fwd_vec1, fwd_vec2)/(sz1*sz2));
}

void RotatePoints(sf::VertexArray& points, double angle, const sf::Vector2f& point)
{
    size_t size = points.getVertexCount();
    for(size_t i = 0; i < size; i++)
    {
        auto pos = points[i].position;
        points[i].position.x = (pos.x - point.x)*cos(angle) - (pos.y - point.y)*sin(angle) + point.x;
        points[i].position.y = (pos.x - point.x)*sin(angle) + (pos.y - point.y)*cos(angle) + point.y;
    }
}

void ScalePointsX(sf::VertexArray& points, double factor)
{
    size_t size = points.getVertexCount();
    for(size_t i = 0; i < size; i++)
    {
        points[i].position.x *= factor;
    }
}

bool IsPointInCircle(const sf::Vector2f& point, const Circle& circle)
{
    return ((point.x - circle.first.x)*(point.x - circle.first.x) + (point.y - circle.first.y)*(point.y - circle.first.y)) <= circle.second * circle.second;
}

sf::Vector2f GetBboxCenter(const sf::FloatRect& bbox)
{
    sf::Vector2f left_bottom{bbox.left, bbox.top},
                right_upper{left_bottom.x + bbox.width, left_bottom.y + bbox.height};
    return {(left_bottom.x + right_upper.x)/2, (left_bottom.y + right_upper.y)/2};
}

std::pair<sf::VertexArray, sf::VertexArray> SplitPoints(const sf::VertexArray& points)
{
    size_t size = points.getVertexCount();
    sf::VertexArray left, right;
    auto bbox = points.getBounds();
    sf::Vector2f center = GetBboxCenter(bbox);
    for(size_t i = 0; i < size; ++i)
    {
        if(points[i].position.x <= center.x)
        {
            left.append(points[i]);
        }
        else
        {
            right.append(points[i]);
        }
    }
    if(left.getVertexCount() != 0 && right.getVertexCount() != 0)
        return {std::move(left), std::move(right)};
    left.clear();
    right.clear();
    for(size_t i = 0; i < size; ++i)
    {
        if(points[i].position.y <= center.y)
        {
            left.append(points[i]);
        }
        else
        {
            right.append(points[i]);
        }
    }
    return {std::move(left), std::move(right)};
}

inline double GetBboxRadius(const sf::FloatRect& bbox, const sf::Vector2f& bbox_center)
{
    return GetDistance2D({bbox.left, bbox.top}, bbox_center);
}

double GetM(const std::pair<sf::VertexArray, sf::VertexArray>& pair)
{
    auto bbox_u = pair.first.getBounds();
    auto bbox_v = pair.second.getBounds();
    sf::Vector2f u_center = GetBboxCenter(bbox_u),
                v_center = GetBboxCenter(bbox_v);
    return GetDistance2D(u_center, v_center) + GetBboxRadius(bbox_u, u_center) + GetBboxRadius(bbox_v, v_center);
}

bool TryAddPair(std::list<PairDesc>& pairs, std::pair<sf::VertexArray, sf::VertexArray> pair)
{
    if(pair.first.getVertexCount() == 0 || pair.second.getVertexCount() == 0)
        return false;
    bool bInserted = false;
    PairDesc p{GetM(pair), std::move(pair)};
    for(auto b = pairs.begin(); b != pairs.end(); ++b)
    {
        if(b -> M < p.M)
        {
            pairs.insert(b, std::move(p));
            bInserted = true;
            break;
        }
    }
    if(!bInserted)
    {
        pairs.push_back(std::move(p));
    }
    return true;
}

double GetMaxDistanceSariel(Line& max, const sf::VertexArray& points)
{
    std::list<PairDesc> pairs;
    if(points.getVertexCount() == 0)
        return 0.;
    auto pair = std::move(SplitPoints(points));
    if(!TryAddPair(pairs, pair))
        return 0.;
    max = {pair.first[0].position, pair.second[0].position};
    double max_dist = GetDistance2D(max.first, max.second);
    while(!pairs.empty())
    {
        const PairDesc& p = pairs.front();
        if(p.M <= (1 + EPS_SARIEL)*max_dist)
        {
            break;
        }
        auto first_p = std::move(SplitPoints(p.pair.first));
        auto second_p = std::move(SplitPoints(p.pair.second));
        if( (first_p.first.getVertexCount() == 0 || first_p.second.getVertexCount() == 0) &&
            (second_p.first.getVertexCount() == 0 || second_p.second.getVertexCount() == 0) )
        {
            continue;
        }
        pairs.pop_front();
        double cur_dist = 0.;
        if(TryAddPair(pairs, std::move(std::pair<sf::VertexArray, sf::VertexArray>{first_p.first, second_p.first})))
        {
            cur_dist = GetDistance2D(first_p.first[0].position, second_p.first[0].position);
            if(max_dist < cur_dist)
            {
                max_dist = cur_dist;
                max = {first_p.first[0].position, second_p.first[0].position};
            }
        }
        if(TryAddPair(pairs, std::move(std::pair<sf::VertexArray, sf::VertexArray>{first_p.first, second_p.second})))
        {
            cur_dist = GetDistance2D(first_p.first[0].position, second_p.second[0].position);
            if(max_dist < cur_dist)
            {
                max_dist = cur_dist;
                max = {first_p.first[0].position, second_p.second[0].position};
            }
        }
        if(TryAddPair(pairs, std::move(std::pair<sf::VertexArray, sf::VertexArray>{first_p.second, second_p.first})))
        {
            cur_dist = GetDistance2D(first_p.second[0].position, second_p.first[0].position);
            if(max_dist < cur_dist)
            {
                max_dist = cur_dist;
                max = {first_p.second[0].position, second_p.first[0].position};
            }
        }
        if(TryAddPair(pairs, std::move(std::pair<sf::VertexArray, sf::VertexArray>{first_p.second, second_p.second})))
        {
            cur_dist = GetDistance2D(first_p.second[0].position, second_p.second[0].position);
            if(max_dist < cur_dist)
            {
                max_dist = cur_dist;
                max = {first_p.second[0].position, second_p.second[0].position};
            }
        }
    }
    return max_dist;
}

std::vector<sf::Vector2f> GetPointsNotInCircle(const sf::VertexArray& points, const Circle& circle)
{
    size_t size = points.getVertexCount();
    std::vector<sf::Vector2f> res;
    for(size_t i = 0; i < size; ++i)
    {
        if(!IsPointInCircle(points[i].position, circle))
        {
            res.push_back(points[i].position);
        }
    }
    return res;
}

double GetMaxDistanceFromPoint(Line& max, const sf::VertexArray& points, const sf::Vector2f& point)
{
    double max_dist = 0.;
    size_t size = points.getVertexCount();
    for(size_t i = 0; i < size; ++i)
    {
        double cur_dist = GetDistance2D(points[i].position, point);
        if(max_dist < cur_dist)
        {
            max_dist = cur_dist;
            max = {points[i].position, point};
        }
    }
    return max_dist;
}

double DoubleNormal(Line& max, sf::Vector2f p, const sf::VertexArray& points)
{
    double dist = 0.;
    double cur_dist = 0.;
    while(true)
    {
        cur_dist = GetMaxDistanceFromPoint(max, points, p);
        if(cur_dist > dist)
        {
            dist = cur_dist;
            p = max.first;
        }
        else break;
    }
    return dist;
}

double GetMaxDistanceMalandain(Line& max, const sf::VertexArray& points)
{
    size_t size = points.getVertexCount();
    if(!size) 
        return 0.;
    double max_dist = 0.;
    sf::Vector2f p = points[0].position;
    while(true)
    {
        Line cur_max;
        double cur_dist = DoubleNormal(cur_max, p, points);
        if(max_dist < cur_dist)
        {
            max_dist = cur_dist;
            max = cur_max;
            std::vector<sf::Vector2f> not_in_circle = std::move(GetPointsNotInCircle(points, {{(max.first.x + max.second.x)/2, (max.first.y + max.second.y)/2}, max_dist/2}));
            if(not_in_circle.size() == 0)
                break;
            p = not_in_circle[0];
        }
        else break;
    }
    return max_dist;
}

double GetMaxDistanceBrute(Line& max, const sf::VertexArray& points)
{
    size_t size = points.getVertexCount();
    double dist = 0.;
    for(size_t i = 0; i < size; i++)
    {
        Line cur_max;
        double cur_dist = GetMaxDistanceFromPoint(cur_max, points, points[i].position);
        if(dist < cur_dist)
        {
            dist = cur_dist;
            max = cur_max;
        }
    }
    return dist;
}

int main()
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::knuth_b g(seed);
    unsigned L1, L2, size;
    std::cout << "Rectangle height: ";
    std::cin >> L1;
    std::cout << "Rectangle width: ";
    std::cin >> L2; 
    std::cout << "Number of points: ";
    std::cin >> size;

    std::uniform_real_distribution<> hrand(POINTS_SHIFT, POINTS_SHIFT + L1), wrand(POINTS_SHIFT, POINTS_SHIFT + L2);
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH_SHIFT + L2, WINDOW_HEIGHT_SHIFT + L1), "Petunin`s ellipses");
    sf::VertexArray points(sf::PrimitiveType::Points, size);
    sf::RectangleShape points_bound({L2, L1});
    points_bound.setFillColor(sf::Color::Transparent);
    points_bound.setOutlineColor(sf::Color::Cyan);
    points_bound.setOutlineThickness(1);
    points_bound.setOrigin({L2/2, L1/2});
    points_bound.setPosition({POINTS_SHIFT + L2/2, POINTS_SHIFT + L1/2});
    for(size_t i = 0; i < size; ++i)
    {
        points[i] = sf::Vertex{{wrand(g), hrand(g)}};
    }
    double a,b;
    Line max;
    b = GetMaxDistanceMalandain(max, points);
    sf::VertexArray max_l(sf::PrimitiveType::Lines, 2);
    max_l[0] = max.first;
    max_l[1] = max.second;
    Line left_bound{max}, right_bound{max};
    double dist = 0.;
    double dist_rigth = 0.;
    for(size_t i = 0; i < size; i++)
    {
        auto point = GetLineNormalIntersectPoint(max, points[i].position);
        double cur_dist = GetDistance2D(points[i].position, point);
        if(IsPointLeftToLine(max, points[i].position))
        {
            if(dist < cur_dist)
            {
                dist = cur_dist;
                left_bound = {max.first + points[i].position - point, max.second + points[i].position - point};
            }
        }
        else
        {
            if(dist_rigth < cur_dist)
            {
                dist_rigth = cur_dist;
                right_bound = {max.first + points[i].position - point, max.second + points[i].position - point};
            }
        }
    }
    a = dist + dist_rigth;
    sf::Vector2f rot_p = {POINTS_SHIFT, POINTS_SHIFT + L2};
    double angle = GetAngle(max, {{0., 0.}, {1., 0.}});
    angle = angle > M_PI_2 ? M_PI - angle : -angle;
    RotatePoints(points, angle, rot_p);
    sf::VertexArray bottom_line(sf::PrimitiveType::Lines, 2), top_line(sf::PrimitiveType::Lines, 2);
    bottom_line[0] = left_bound.first;
    bottom_line[1] = left_bound.second;
    top_line[0] = right_bound.first;
    top_line[1] = right_bound.second;
    RotatePoints(bottom_line, angle, rot_p);
    RotatePoints(top_line, angle, rot_p);
    RotatePoints(max_l, angle, rot_p);
    ScalePointsX(points, a/b);
    ScalePointsX(bottom_line, a/b);
    ScalePointsX(top_line, a/b);
    ScalePointsX(max_l, a/b);
    left_bound = {bottom_line[0].position, bottom_line[1].position};
    right_bound = {top_line[0].position, top_line[1].position};
    sf::Vector2f cube_center = {(left_bound.first.x + left_bound.second.x)/2, (left_bound.first.y + right_bound.second.y)/2};
    std::vector<double> circles;
    std::vector<std::pair<sf::CircleShape, unsigned>> ellipses;
    for(size_t i = 0; i < size; i++)
        circles.push_back(GetDistance2D(cube_center, points[i].position));
    std::sort(circles.begin(), circles.end(),
    [](auto first, auto second)
    {
        return first < second;
    });
    
    cube_center.x *= b/a;
    sf::VertexArray p(sf::PrimitiveType::Points, 1);
    p[0] = cube_center;
    RotatePoints(p, -angle, rot_p);
    unsigned points_count = 0;
    for(size_t i = 0; i < size; i++)
    {
        ++points_count;
        if(i < (size - 1) && fabs(circles[i] - circles[i + 1]) <= MAX_DELTA)
            continue;
        double radius = circles[i];
        sf::CircleShape ellipse(radius);
        ellipse.setOrigin(radius, radius);
        ellipse.setFillColor(sf::Color::Transparent);
        ellipse.setOutlineThickness(0.5);
        ellipse.setOutlineColor(sf::Color::Red);
        ellipse.setPosition(p[0].position);
        ellipse.setScale({b/a, 1.});
        ellipse.rotate(RadToDeg(-angle));
        ellipses.push_back(std::pair<sf::CircleShape, unsigned>{ellipse, points_count});
    }
    ScalePointsX(points, b/a);
    RotatePoints(points, -angle, rot_p);
    ScalePointsX(top_line, b/a);
    RotatePoints(top_line, -angle, rot_p);
    ScalePointsX(bottom_line, b/a);
    RotatePoints(bottom_line, -angle, rot_p);
    ScalePointsX(max_l, b/a);
    RotatePoints(max_l, -angle, rot_p);
    for(const auto& ellipse:ellipses)
    {
        std::cout << "Points: " << ellipse.second << "; Area: " << M_PI * ellipse.first.getRadius()*ellipse.first.getRadius()*b/a<< std::endl;
    }
    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed || 
            (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) window.close();
        }
        window.clear();
        window.draw(points);
        window.draw(bottom_line);
        window.draw(top_line);
        window.draw(max_l);
        window.draw(points_bound);
        for(const auto& ellipse:ellipses)
        {
            window.draw(ellipse.first);
        }
        window.display();
        std::this_thread::sleep_for(std::chrono::duration<int, std::ratio<1>>{});
    }
}